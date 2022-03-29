import RPi.GPIO as GPIO
import sys
import numpy
import subprocess
from main_header import *
from mux import *
from sht30_main_header import *
import sht30
from pa1616_main_header import *
import pa1616
from lora_e5 import *
from camera_main_header import *
from camera_pipeline import *
from typing import *


def main() -> None:
    # Variables for stages and valid flags
    stage: int = 0
    valid_flags: int = 0b1111   # L->R: GPS, TH, Camera, LoRa
    
    # File descriptors
    GPS_File: int
    TH_File: int
    LoRa_File: int

    # Set program to read pin numbers of Raspberry Pi Zero for GPIO
    GPIO.setmode(GPIO.BOARD)

    # Set up error log files
    stderr_fo = sys.stderr
    errors_fo = open(ERRORS_LOG_NAME, "r+")
    gpsErrors_fo = open(GPS_ERRORS_LOG_NAME, "r+")

    # Direct errors to GPS error log file
    sys.stderr = gpsErrors_fo
    
    # Configure GPS switch pin and set to low
    GPS_GPIO_Init()
    switchGPSInit()

    # Configure UART Mux select pins
    muxSelInit()

    if stage == 0:
        # Data containers for stage 0
        TH_buffer: int = [0 for i in range(TH_DATA_SIZE)]

        # Turn GPS switch off, flip mux input to GPS input, turn GPS switch on, and reset the GPS
        switchGPSOff()
        setMuxSel(GPS_MUX_SEL)
        switchGPSOn()
        GPSReset()

        # Configure GPS port and open it
        if (GPS_File := pa1616.openGPSPort(UART_PORT)) < 0:
            clear_valid_flag(GPS_VALID_MASK)

        # Enable GPS antenna
        if (not pa1616_pyobj.enableAntenna(GPS_File)):
            pa1616_pyobj.closeGPSPort(GPS_File)
            clear_valid_flag(GPS_VALID_MASK)
        
        # Direct errors to main error log file
        sys.stderr = errors_fo

        # Configure I2C-1 port, initialize T/H sensor, and run test
        if ((TH_File := sht30.initialize(TH_BUS)) < 0) or (not TH_Test(TH_File, TH_buffer)):
            clear_valid_flag(TH_VALID_MASK)

        # Check that camera is detected
        if not detectCamera():
            clear_valid_flag(CAMERA_VALID_MASK)

        # Flip mux input to LoRa input, initialize LoRa, and flip mux input back to GPS input
        setMuxSel(LORA_MUX_SEL)
        if ((LoRa_File := init_LoRa(UART_PORT)) < 0) or (not test_LoRa()):
            clear_valid_flag(LORA_VALID_MASK)
        setMuxSel(GPS_MUX_SEL)

        stage = 1

    if stage == 1:
        # Data containers for stage 1
        GPS_Data: list[str] = ["".join('a' for i in range(GPS_MSG_SIZE)) for i in range(2)]
        GPS_Fields: list[str] = ["" for i in range(GPS_PARSED_MSG_NUM_FIELDS)]
        GPS_Pkg = pa1616.GPSPkg()
        GGA_flag: int = 0
        RMC_flag: int = 0

        # Direct errors to gps-errors.log
        sys.stderr = gpsErrors_fo

        while not (GGA_flag and RMC_flag):
            # Obtain fix from GPS
            if check_valid_flag(GPS_VALID_MASK) and \
            (pa1616.obtainFix(GPS_File, GPS_Data, GGA_flag, RMC_flag) < 0):
                clear_valid_flag(GPS_VALID_MASK)
                break
            
            #***Do checksum valid in obtainfix or no?
            #***Check if latitude and longitude exist (if not, repeat obtain fix?)
            
            # Package GPS data into a struct
            if check_valid_flag(GPS_VALID_MASK) and \
            (pa1616.packageGPSData(GPS_Data, GPS_Fields, GPS_Pkg) < 0):
                clear_valid_flag(GPS_VALID_MASK)
                break

            # Set GGA and RMC flags
            if (GPS_Fields[0][GPS_RMC_START_IDX:] == RMC_STR):
                RMC_flag = 1
            else:
                GGA_flag = 1
            
            # Set time according to time obtained from GPS (if GPS data format is RMC)
            if check_valid_flag(GPS_VALID_MASK) and RMC_flag:
                pa1616.setTime(GPS_Fields[RMC_DATE_IDX], GPS_Fields[RMC_TIME_IDX])

        # Send gps-errors.log over LoRa
        gpsErrorString: str = gpsErrors_fo.read().decode('UTF-8')
        if check_valid_flag(LORA_VALID_MASK):
            send_LoRa(gpsErrorString, LoRa_File)

        # Switch off GPS and direct errors to errors.log
        pa1616.closeGPSPort(GPS_File);
        switchGPSOff()
        sys.stderr = errors_fo
        gpsErrors.close()

        # Send GPS data over LoRa
        gpsDataString: str = pack_data(GPS_FORMAT, [GPS_Pkg.latitude, GPS_Pkg.longitude])
        if check_valid_flag(LORA_VALID_MASK):
            send_LoRa(gpsDataString, LoRa_File)

        stage = 2

    if stage == 2:
        # Data containers for stage 2
        label_list: list[str]
        TH_Data: list[float] = [0.0 for i in range(TH_NUM_FIELDS)]
        temp: int
        errorString: str
        dataString: str

        while 1:
            # Obtain classification labels from camera
            if check_valid_flag(CAMERA_VALID_MASK):
                label_list = classification_pipeline()

            # Obtain temperature/humidity data from TH sensor
            if check_valid_flag(TH_VALID_MASK):
                if sht30.processData(TH_File, temp, TH_Data) < 0:
                    clear_valid_flag(TH_VALID_MASK)
            
            # Send errors.log over LoRa
            errorString = errors_fo.read().decode('UTF-8')
            if check_valid_flag(LORA_VALID_MASK):
                send_LoRa(errorString, LoRa_File)
            errors_fo.truncate(0)

            # Send data string over LoRa
            dataString = pack_data(DATA_FORMAT, [temp] + \
                                   [TH_Data[i] for i in range(TH_NUM_FIELDS)] + \
                                   [label_list[i] if i < len(label_list) else "" \
                                    for i in range(MAX_LABEL_LIST_LEN)])
            if check_valid_flag(LORA_VALID_MASK):
                send_LoRa(dataString, LoRa_File)

            # Put LoRa into sleep mode
            sleep_LoRa()

            #***set timer for 2 minutes
            #***enter deep sleep mode

            #***wake up MCU
            awake_LoRa()

    # Close errors.log
    sys.stderr = stderr_fo
    errors_fo.close()

main()