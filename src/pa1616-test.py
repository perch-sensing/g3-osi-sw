import pa1616_pyobj
from timer import *
from typing import *
import sys
import RPi.GPIO as GPIO
from time import sleep

DEBUG: int = 1    # print results to console
DEBUG2: int = 0   # verbose debugging statements
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
GGA_RMC_flags: int = 0b00
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

GPS_READ_TIME_LIMIT: int = 15
GPS_SW_EN_PIN: int = 35
GPS_NRESET_PIN: int = 37
GPS_FIX_PIN: int = 40
FIX_TIMER_COUNTER: int = 32
FIX_TIMER_BUFFER_MASK: int = 0x000000FF

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
GPS_MUX_SEL: tuple[int, int] = (0, 1)

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

def main() -> None:
    stderr_fileno: int = sys.stderr
    sys.stderr = open("errors.log", "w")

    fd: int
    buffer: str = str(GPS_DATA_MSG)
    fields = ["" for i in range(GPS_PARSED_MSG_NUM_FIELDS)]
    GPS_Valid: int = 1
    gps = pa1616_pyobj.GPSPkg()

    if buffer[3:6] == GGA_STR:
        if (pa1616_pyobj.packageGPSData(buffer, fields, gps, GGA_LAT_IDX, GGA_LON_IDX) < 0):
            GPS_Valid = 0
            return -1

        if DEBUG:
            print("UTC Time  :", fields[1].decode('UTF-8'))
            print("Latitude  :", gps.latitude)
            print("Longitude :", gps.longitude)
            print("Altitude  :", fields[9].decode('UTF-8'))
            print("Satellites:", fields[7].decode('UTF-8'))

    if buffer[3:6] == RMC_STR:
        if DEBUG:
            print("Speed     :", fields[7].decode('UTF-8'))
            print("UTC Time  :", fields[1].decode('UTF-8'))
            print("Date      :", fields[9].decode('UTF-8'))
        
        if (pa1616_pyobj.setTime(fields[RMC_DATE_IDX].decode('UTF-8'), fields[RMC_TIME_IDX].decode('UTF-8')) < 0):
            GPS_Valid = 0
        
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    GPS_GPIO_Init()
    switchGPSInit()
    switchGPSOff()
    muxSelInit()
    setMuxSel(GPS_MUX_SEL)
    switchGPSOn()
    GPSReset()
    
    #GPIO.cleanup()
    #return

    buff: list[str]
    fd = pa1616_pyobj.openGPSPort(UART_DEV)
    sleep(30)
    if (not pa1616_pyobj.enableAntenna(fd)):
        pa1616_pyobj.closeGPSPort(fd)
        return -1
    #sleep(200)
    '''sys.stderr.close()
    sys.stderr = stderr_fileno
    return'''

    if not waitForFix():
        pa1616_pyobj.closeGPSPort(fd)
        return -1
    
    GGA_set: int = 0
    RMC_set: int = 0

    if (fd > 0):
        with Timer() as t:
            t.start()
            while t.lap() < GPS_READ_TIME_LIMIT:
                if not (~GGA_RMC_flags & (GGA_MASK | RMC_MASK)):
                    break
                buff = ["".join('a' for i in range(GPS_MSG_SIZE)) for j in range(2)]

                if (pa1616_pyobj.obtainFix(fd, buff) < 0):
                    continue
                buffer: str
                counter: int = 0
                if ((buffer := extractMsg(buff[0])) == ""):
                    continue
                if DEBUG:
                    print("buffer (python):", buffer)
                    print("flags:", GGA_RMC_flags)
                if (pa1616_pyobj.checksum_valid(buffer) < 0):
                    continue
                if (GGA_RMC_flags & GGA_MASK and not GGA_set):
                    if (pa1616_pyobj.packageGPSData(buffer, fields, gps, GGA_LAT_IDX, GGA_LON_IDX) < 0):
                        GPS_Valid = 0
                        continue
                    GGA_set = 1
                    if DEBUG:
                        print("Latitude  :", gps.latitude)
                        print("Longitude :", gps.longitude)

                if (GGA_RMC_flags & RMC_MASK and not RMC_set):
                    if (pa1616_pyobj.parse_comma_delimited_str(buffer, fields, GPS_PARSED_MSG_NUM_FIELDS) < 0):
                        GPS_Valid = 0
                        continue
                    if (pa1616_pyobj.setTime(fields[RMC_DATE_IDX].decode('UTF-8'), fields[RMC_TIME_IDX].decode('UTF-8')) < 0):
                        pass
                    RMC_set = 1

    if pa1616_pyobj.disableAntenna(fd):
        pa1616_pyobj.closeGPSPort(fd)
        return -1
                                                              
    if pa1616_pyobj.closeGPSPort(fd):
        GPS_Valid = 0
        return -1
    
    #switchGPSOff()
    #GPIO.cleanup()
    sys.stderr.close()
    sys.stderr = stderr_fileno

main()
