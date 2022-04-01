from typing import *
import sys
import RPi.GPIO as GPIO
from time import sleep

DEBUG: int = 1    # print results to console
DEBUG2: int = 1   # verbose debugging statements
GPS_MSG_SIZE: int = 350
GPS_PARSED_MSG_NUM_FIELDS: int = 20
GGA_STR: str = "GGA"
RMC_STR: str = "RMC"
GP_STR: str = "$GP"
GN_STR: str = "$GN"
GGA_MASK: int = 0b10
RMC_MASK: int = 0b01
GGA_RET_VALUE: int = 0
RMC_RET_VALUE: int = 1
GGA_VALID_IDX: int = 6
RMC_VALID_IDX: int = 2
GGA_LAT_IDX: int = 2
GGA_LON_IDX: int = 4
RMC_LAT_IDX: int = 3
RMC_LON_IDX: int = 5
RMC_DATE_IDX: int = 9
RMC_TIME_IDX: int = 1

GPS_READ_TIME_LIMIT: int = 20
GPS_SW_EN_PIN: int = 35
GPS_NRESET_PIN: int = 37
GPS_FIX_PIN: int = 40
FIX_TIMER_COUNTER: int = 32
FIX_TIMER_BUFFER_MASK: int = 0x000000FF

#Global variables
GGA_RMC_flags: int = 0b00

def GPS_GPIO_Init() -> None:
    GPIO.setup(GPS_NRESET_PIN, GPIO.OUT)
    GPIO.setup(GPS_FIX_PIN, GPIO.IN)

def GPSReset() -> None:
    GPIO.output(GPS_NRESET_PIN, GPIO.LOW)
    sleep(2)
    GPIO.output(GPS_NRESET_PIN, GPIO.HIGH)

def switchGPSInit() -> None:
    GPIO.setup(GPS_SW_EN_PIN, GPIO.OUT)

def switchGPSOn() -> None:
    GPIO.output(GPS_SW_EN_PIN, GPIO.LOW)

def switchGPSOff() -> None:
    GPIO.output(GPS_SW_EN_PIN, GPIO.HIGH)

def waitForFix() -> bool:
    buffer: int = 0xFFFFFFFF
    for i in range(FIX_TIMER_COUNTER):
        readIn = GPIO.input(GPS_FIX_PIN)
        buffer = (buffer << 1) + readIn
        if DEBUG2:
            print("fix pin:", readIn)
        if (buffer & FIX_TIMER_BUFFER_MASK) == 0:
            if DEBUG2:
                 print("\n\n\nFix obtained!\n\n\n")
            return True
        sleep(0.5)
    print("PA1616: Fix waiting timeout.", file=sys.stderr)
    return False

'''
    Check if NMEA message contains valid data

    @param {str} c - validity indicator of the message
    @param {int} fieldIdx - index of validity indicator in message

    @return {bool} true if data is valid, false otherwise
'''
def validCheck(c: str, fieldIdx: int) -> bool:
	return (c == '1') if (fieldIdx == GGA_VALID_IDX) else (c == 'A')

'''
   Extract valid NMEA message from the buffer
   
   @param {str} buffString - string object of the buffer
   #@param {int} flags - flags to indicate GGA, RMC, or either GGA or RMC message type
    
   @return {str} valid NMEA message, or empty string if no valid NMEA message
'''
def extractMsg(buffString: str) -> str:#, flags: int) -> str:
    beginIndex: int = 0
    global GGA_RMC_flags
    if (DEBUG):
        print("buffString size (extractMsg):", len(buffString))
        print("flags:", GGA_RMC_flags)
    while ((beginIndex := buffString.find('$', beginIndex)) >= 0):
        if (DEBUG):
            print("beginIndex:", beginIndex)
        if buffString.find("GN", beginIndex, beginIndex + 3) > 0 or buffString.find("GP", beginIndex, beginIndex + 3) > 0:
            if (DEBUG):
                print("GN or GP detected.")
            GGA_Flag: int = -1
            RMC_Flag: int = -1
            if (not (GGA_RMC_flags & GGA_MASK) and (GGA_Flag := buffString.find("GGA", beginIndex, beginIndex + 6)) > 0) or \
                (not (GGA_RMC_flags & RMC_MASK) and (RMC_Flag := buffString.find("RMC", beginIndex, beginIndex + 6)) > 0):
                if (DEBUG):
                    print("GGA or RMC detected.")
                    print("GGA_Flag:", GGA_Flag) 
                    print("RMC_Flag:", RMC_Flag)
                endIndex: int
                if (endIndex := buffString.find(chr(10), beginIndex)) > 0:
                    fieldIdx: int
                    if GGA_Flag > 0:
                        fieldIdx = GGA_VALID_IDX
                        GGA_RMC_flags |= GGA_MASK
                        if DEBUG2:
                            print("flags:", GGA_RMC_flags)
                    else:
                        fieldIdx = RMC_VALID_IDX
                        GGA_RMC_flags |= RMC_MASK
                        if DEBUG2:
                            print("flags:", GGA_RMC_flags)
                    iterIdx: int = beginIndex
                    for i in range(fieldIdx):
                        iterIdx = buffString.find(',', iterIdx)
                        iterIdx += 1
                        if validCheck(buffString[iterIdx], fieldIdx):
                            return buffString[beginIndex:endIndex+1]
        beginIndex += 1
    return str("")