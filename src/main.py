import RPi.GPIO as GPIO
import sys
import numpy
import subprocess
from main_header import *
from mux import *
from sht30_main_header import *
import sht30
import pa1616_main_header
from pa1616_main_header import *
import pa1616
from lora_e5_main_header import *
from lora_e5 import *
from camera_main_header import *
from camera_pipeline import *
from timer import *
from typing import *


def main() -> None:
    # Variables for stages and valid flags
    stage: int = 0
    global valid_flags
    
    # File descriptors
    GPS_File: int
    TH_File: int
    LoRa_File: Type[serial.Serial]

    # Set program to read pin numbers of Raspberry Pi Zero for GPIO
    GPIO.setmode(GPIO.BOARD)

    # Set up error log files
    stderr_fo = sys.stderr
    errors_fo = open(ERRORS_LOG_NAME, "w+")
    gpsErrors_fo = open(GPS_ERRORS_LOG_NAME, "w+")

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
        #switchGPSOff()
        print("***Switch TX to GPS***")
        sleep(10)
        setMuxSel(GPS_MUX_SEL)
        #switchGPSOn()
        #GPSReset()

        # Configure GPS port and open it
        if (GPS_File := pa1616.openGPSPort(UART_PORT)) < 0:
            clear_valid_flag(GPS_VALID_MASK)

        # Enable the GPS antenna
        if check_valid_flag(GPS_VALID_MASK) and not pa1616.enableAntenna(GPS_File):
            pa1616.closeGPSPort(GPS_File)
            clear_valid_flag(GPS_VALID_MASK)

        # Wait 120 seconds for letting the GPS to wake up
        #sleep(120)
        
        # Direct errors to main error log file
        sys.stderr = errors_fo

        # Configure I2C-1 port, initialize T/H sensor, and run test
        if ((TH_File := sht30.initialize(TH_BUS)) < 0) or (not TH_Test(TH_File, TH_buffer)):
            clear_valid_flag(TH_VALID_MASK)

        # Check that camera is detected
        if not detectCamera():
            clear_valid_flag(CAMERA_VALID_MASK)

        # Flip mux input to LoRa input, initialize LoRa, and flip mux input back to GPS input
        print("***Switch TX to LoRa***")
        sleep(10)
        setMuxSel(LORA_MUX_SEL)
        if (not (LoRa_File := init_LoRa(UART_PORT))) or (not test_LoRa(LoRa_File)):
            clear_valid_flag(LORA_VALID_MASK)

        #***receive credentials for LoRa session through the LOAD input of the UART mux***

        print("***Switch TX to GPS***")
        sleep(10)
        setMuxSel(GPS_MUX_SEL)

        stage = 1

    cntr: int = 0
    while cntr < 4:
        print("\n\n***** Loop", cntr + 1, "*****\n")
        if stage == 1:
            print("\n*** stage", stage, "***")
            # Data containers for stage 1
            GPS_Pkg = pa1616.GPSPkg()

            GGA_set: int = 0
            RMC_set: int = 0

            # Direct errors to GPS error log file
            sys.stderr = gpsErrors_fo

            if check_valid_flag(GPS_VALID_MASK):
                # Wait for the GPS to get a fix over a certain period of time
                if not waitForFix():
                    clear_valid_flag(GPS_VALID_MASK)
                else:
                    with Timer() as t:
                        t.start()
                        curr_lap: int = t.lap()
                        # Attempt to acquire a GGA message (for latitude and longitude) and an RMC message (for system time and date) in a certain time frame
                        while (curr_lap := t.lap()) < GPS_READ_TIME_LIMIT:
                            print("** curr_lap =", curr_lap, "**")
                            print("** GGA_RMC_flags", pa1616_main_header.GGA_RMC_flags, "**")
                            sleep(3)
                            if not (~pa1616_main_header.GGA_RMC_flags & (GGA_MASK | RMC_MASK)):
                                break
                            GPS_Data: list[str] = ["".join(' ' for i in range(GPS_MSG_SIZE)) for j in range(2)]
                            GPS_Fields: list[str] = ["" for i in range(GPS_PARSED_MSG_NUM_FIELDS)]

                            # Obtain a buffer of messages
                            if (pa1616.obtainFix(GPS_File, GPS_Data) < 0):
                                continue
                            GPS_Buffer: str
                            counter: int = 0

                            # Extract a valid GGA or RMC message (if such is available)
                            if ((GPS_Buffer := extractMsg(GPS_Data[0])) == ""):
                                continue
                            if (pa1616.checksum_valid(GPS_Buffer) < 0):
                                continue

                            # If the message is GGA and a GGA message has not been received prior, extract the latitude and longitude from it and store it in the GPS struct
                            if (pa1616_main_header.GGA_RMC_flags & GGA_MASK and not GGA_set):
                                if (pa1616.packageGPSData(GPS_Buffer, GPS_Fields, GPS_Pkg, GGA_LAT_IDX, GGA_LON_IDX) < 0):
                                    pa1616_main_header.GGA_RMC_flags &= ~GGA_MASK
                                    continue
                                GGA_set = 1

                            # If the message is RMC and an RMC message has not been received prior, extract the date and time from it and set the system clock with this data
                            if (pa1616_main_header.GGA_RMC_flags & RMC_MASK and not RMC_set):
                                if (pa1616.parse_comma_delimited_str(GPS_Buffer, GPS_Fields, GPS_PARSED_MSG_NUM_FIELDS) < 0):
                                    pa1616_main_header.GGA_RMC_flags &= ~RMC_MASK
                                    continue
                                if (pa1616.setTime(GPS_Fields[RMC_DATE_IDX], GPS_Fields[RMC_TIME_IDX]) < 0):
                                    pass
                                RMC_set = 1

                        # Check whether the timer reached the end and clear the corresponding flag if messages were not obtained within the given time frame
                        if curr_lap >= GPS_READ_TIME_LIMIT:
                            clear_valid_flag(GPS_VALID_MASK)

                pa1616.disableAntenna(GPS_File)

            # Switch off GPS, direct errors to main error log file, and flip mux input to LoRa input
            #if check_valid_flag(GPS_VALID_MASK):
                #pa1616.closeGPSPort(GPS_File)
                #switchGPSOff()
            sys.stderr = errors_fo
            print("***Switch TX to LoRa***")
            sleep(10)
            setMuxSel(LORA_MUX_SEL)

            # Extract contents from GPS error log file, close the file, and send GPS error log file
            # over LoRa
            gpsErrorBytes: Type[bytes] = bytes(gpsErrors_fo.read(), "UTF-8")
            if check_valid_flag(GPS_VALID_MASK):
                gpsErrors_fo.close()
            if check_valid_flag(LORA_VALID_MASK):
                send_LoRa(gpsErrorBytes, LoRa_File)

            # Send GPS data over LoRa
            gpsDataBytes: Type[bytes] = pack_data([GPS_Pkg.latitude, GPS_Pkg.longitude])
            if check_valid_flag(LORA_VALID_MASK):
                send_LoRa(gpsDataBytes, LoRa_File)

            stage = 2

        if stage == 2:
            print("\n*** stage", stage, "***")
            # Data containers for stage 2
            label_list: list[str] = [""]
            TH_buffer: int = [0 for i in range(TH_DATA_SIZE)]
            TH_Data: list[float] = [0.0 for i in range(TH_NUM_FIELDS)]
            temp: list[int] = [0 for i in range(2)]
            errorBytes: Type[bytes]
            dataBytes: Type[bytes]
            ctr = 0

            while ctr < 1:
                # Obtain classification labels from camera
                if check_valid_flag(CAMERA_VALID_MASK):
                    label_list = classification_pipeline()

                # Obtain temperature/humidity data from TH sensor
                if check_valid_flag(TH_VALID_MASK):
                    if sht30.processData(TH_File, TH_buffer, temp, TH_Data) < 0:
                        clear_valid_flag(TH_VALID_MASK)

                # Send main error log file over LoRa
                errorBytes = bytes(errors_fo.read(), 'UTF-8')
                if check_valid_flag(LORA_VALID_MASK):
                    send_LoRa(errorBytes, LoRa_File)
                errors_fo.truncate(0)

                # Send data over LoRa
                dataBytes = pack_data([str(temp[0])] + \
                                       ["{:.2f}".format(TH_Data[i]) for i in range(TH_NUM_FIELDS)] + \
                                       [label_list[i] for i in range(len(label_list))])
                if check_valid_flag(LORA_VALID_MASK):
                    send_LoRa(dataBytes, LoRa_File)

                # Put LoRa into sleep mode
                sleep_LoRa(LoRa_File)

                # Sleep for 2 minutes
                sleep(10)

                # Awake LoRa from sleep mode
                awake_LoRa(LoRa_File)
                if not check_valid_flag(GPS_VALID_MASK):
                    print("\n***Trying to read from GPS again.***\n")
                    print("***Switch TX to GPS***")
                    sleep(10)
                    setMuxSel(GPS_MUX_SEL)
                    pa1616.enableAntenna(GPS_File)
                    set_valid_flag(GPS_VALID_MASK)
                    pa1616_main_header.GGA_RMC_flags = GGA_MASK | RMC_MASK
                    stage = 1
                    break

                ctr += 1
        cntr += 1

    GPIO.cleanup()
    # Close main error log file
    sys.stderr = stderr_fo
    errors_fo.close()

main()