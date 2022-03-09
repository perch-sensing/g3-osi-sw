import pa1616_pyobj
from typing import *
import sys
import RPi.GPIO as GPIO
from time import sleep

DEBUG: int = 1    # print results to console
DEBUG2: int = 1   # verbose debugging statements
UART_DEV: str = "/dev/ttyAMA0"
GPS_MSG_SIZE: int = 350
GPS_PARSED_MSG_NUM_FIELDS: int = 20
GPS_DATA_MSG: str = (
    "$GNGGA,165006.000,2241.9107,N,12017.2383,E,1,14,0.79,22.6,M,18.5,M,,*42"
)
GGA_STR: str = "GGA"
RMC_STR: str = "RMC"
GP_STR: str = "$GP"
GN_STR: str = "$GN"

GPS_SW_EN_PIN: int = 35
GPS_NRESET_PIN: int = 37
GPS_FIX_PIN: int = 40
GPS_NUM_READ_ATTEMPTS: int = 3
FIX_TIMER_COUNTER: int = 32
FIX_TIMER_BUFFER_MASK: int = 0x000000FF

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
GPS_MUX_SEL: tuple[int, int] = (0, 1)

def GPS_GPIO_Init() -> None:
    GPIO.setup(GPS_NRESET_PIN, GPIO.OUT)
    GPIO.setup(GPS_FIX_PIN, GPIO.IN)
    GPIO.output(GPS_NRESET_PIN, GPIO.LOW)
    sleep(2)
    GPIO.output(GPS_NRESET_PIN, GPIO.HIGH)

def switchGPSInit() -> None:
    GPIO.setup(GPS_SW_EN_PIN, GPIO.OUT)

def switchGPSOn() -> None:
    GPIO.output(GPS_SW_EN_PIN, GPIO.LOW)

def switchGPSOff() -> None:
    GPIO.output(GPS_SW_EN_PIN, GPIO.HIGH)

def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)

def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])

def waitForFix() -> bool:
    buffer: int = 0xFFFFFFFF
    for i in range(FIX_TIMER_COUNTER):
        readIn = GPIO.input(GPS_FIX_PIN)
        buffer = (buffer << 1) + readIn
        print("fix pin:", readIn)
        if (buffer & FIX_TIMER_BUFFER_MASK) == 0:
            if DEBUG2:
                 print("\n\n\nFix obtained!\n\n\n")
            return True
    print("PA1616: Fix waiting timeout.", file=stderr)
    return False

def main() -> None:
    stderr_fileno: int = sys.stderr
    sys.stderr = open("errors.log", "w")

    fd: int
    buffer: str = str(GPS_DATA_MSG)
    fields = ["" for i in range(GPS_PARSED_MSG_NUM_FIELDS)]
    GPS_Valid: int = 1
    gps = pa1616_pyobj.GPSPkg()

    if buffer[3:6] == GGA_STR:
        if (pa1616_pyobj.packageGPSData(buffer, fields, gps) < 0):
            GPS_Valid = 0
            return -1

        if DEBUG:
            print("UTC Time  :", fields[1].decode('UTF-8'))
            print("Latitude  :", gps.latitude)
            print("Longitude :", gps.longitude)
            print("Altitude  :", fields[9].decode('UTF-8'))
            print("Satellites:", fields[7].decode('UTF-8'))

    if buffer[3:6] == RMC_STR:
        if (pa1616_pyobj.packageGPSData(buffer, fields, gps) < 0):
            GPS_Valid = 0
            return -1

        if DEBUG:
            print("Speed     :", fields[7].decode('UTF-8'))
            print("UTC Time  :", fields[1].decode('UTF-8'))
            print("Date      :", fields[9].decode('UTF-8'))
        
        if (pa1616_pyobj.setTime(fields[9].decode('UTF-8'), fields[1].decode('UTF-8')) < 0):
            GPS_Valid = 0
    
    GPIO.cleanup()    
    GPIO.setmode(GPIO.BOARD)
    GPS_GPIO_Init()
    switchGPSInit()
    switchGPSOff()
    muxSelInit()
    setMuxSel(GPS_MUX_SEL)
    switchGPSOn()

    buff: list[str]
    fd = pa1616_pyobj.openGPSPort(UART_DEV)
    if (fd < 0):
        for i in range(GPS_NUM_READ_ATTEMPTS):
            buff = ["".join('a' for i in range(GPS_MSG_SIZE)) for j in range(2)]
            if (not waitForFix()) or (pa1616_pyobj.obtainFix(fd, buff) < 0):
                continue
            buffer = buff[0]
            counter: int = 0

            if DEBUG:
                print("buffer (python):", buffer)

            if not pa1616_pyobj.checksum_valid(buffer):
                if (buffer[0:3] == GP_STR) or (buffer[0:3] == GN_STR):
                    if buffer[3:6] == GGA_STR:
                        if (pa1616_pyobj.packageGPSData(buffer, fields, gps) < 0):
                            GPS_Valid = 0
                            continue

                        if DEBUG:
                            print("UTC Time  :", fields[1].decode('UTF-8'))
                            print("Latitude  :", gps.latitude)
                            print("Longitude :", gps.longitude)
                            print("Altitude  :", fields[9].decode('UTF-8'))
                            print("Satellites:", fields[7].decode('UTF-8'))

                    if buffer[3:6] == RMC_STR:
                        if (pa1616_pyobj.packageGPSData(buffer, fields, gps) < 0):
                            GPS_Valid = 0
                            continue

                        if DEBUG:
                            print("Speed     :", fields[7].decode('UTF-8'))
                            print("UTC Time  :", fields[1].decode('UTF-8'))
                            print("Date      :", fields[9].decode('UTF-8'))

                        if (pa1616_pyobj.setTime(fields[9].decode('UTF-8'), fields[1].decode('UTF-8')) < 0):
                            GPS_Valid = 0

    if pa1616_pyobj.closeGPSPort(fd):
        GPS_Valid = 0
        return -1

    sys.stderr.close()
    sys.stderr = stderr_fileno

main()