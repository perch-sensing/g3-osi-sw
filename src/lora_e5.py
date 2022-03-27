import RPi.GPIO as GPIO
import serial
import struct
import sys
from typing import *
from time import sleep

LORA_RESET: int = 36
LORA_BOOT: int = 38
BUFF_SIZE: int = 500
BAUD_RATE: int = 9600
PORT_NUM: str = "7"
BAND_PLAN: Type[bytes] = bytes("US915", "UTF-8")
DATA_RATE: Type[bytes] = bytes("DR3", "UTF-8")
CHANNELS: Type[bytes] = bytes("NUM,8-15", "UTF-8")
MODE: Type[bytes] = bytes("LWOTAA", "UTF-8")
DEBUG: int = 1

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
LORA_MUX_SEL: tuple[int, int] = (1, 0)

COMM_HEADER: str = "AT"
COMM_END: str = "\r\n"
DEV_NAME: str = "/dev/ttyAMA0"

'''Initialize mux select
 
   @return {None}	
'''
def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)


'''Set mux select

   @param {tuple[int, int]} sel - mux select configuration
 
   @return {None}	
'''
def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])


'''Create a command

   @param {str} comm - command type
   @param {Type[bytes]) *args - command arguments in bytes
 
   @return {Type[bytes]} command in bytes
'''
def command(comm: str, *args: Type[bytes]) -> Type[bytes]:
    fullCom_str: str = COMM_HEADER

    # append command
    if comm != "":
        fullCom_str = fullCom_str + "+" + comm

    # convert command into bytes object
    fullCom: Type[bytes] = bytes(fullCom_str, "UTF-8")

    # append arguments to the command
    if len(args) > 0:                      
        fullCom = fullCom + bytes("=", "UTF-8") + args[0]
        for i in range(1, len(args)):
            fullCom = fullCom + bytes(" ", "UTF-8") + args[i]

    # append carriage return and newline characters
    fullCom += bytes(COMM_END, "UTF-8")

    if DEBUG:
        print(fullCom)

    return fullCom


'''Write command to serial port (helper function)

   @param {Type[serial.Serial]} serialPort - serial port for LoRa module
   @param {Type[bytes]) comm_bytes - command in bytes
 
   @return {None}	
'''
def init_helper(serialPort: Type[serial.Serial], comm_bytes: Type[bytes]) -> None:
    serialPort.write(comm_bytes)
    serialPort.flush()

    if DEBUG:
        print(recv_LoRa(serialPort))


'''Initialize LoRa

   @param {str} dev_name - name of device file for LoRa module
 
   @return {Type[serial.Serial]} serial port for LoRa module
'''
def init_LoRa(dev_name: str) -> Type[serial.Serial]:
    GPIO.setup(LORA_RESET, GPIO.OUT) 
    GPIO.setup(LORA_BOOT, GPIO.OUT)

    # shift LoRa into AT Application mode
    GPIO.output(LORA_BOOT, GPIO.HIGH)
    sleep(1)

    # reset the LoRa (reset is active low)
    GPIO.output(LORA_RESET, GPIO.LOW)
    sleep(5)
    GPIO.output(LORA_RESET, GPIO.HIGH)

    # open serial port
    serialPort: Type[serial.Serial] = serial.Serial(dev_name, BAUD_RATE, timeout=5)
    if DEBUG:
        print("Opened serial port")

    sleep(5)
    try:
        init_helper(serialPort, command("DR", BAND_PLAN))  
        #init_helper(serialPort, command("DR", DATA_RATE))
        init_helper(serialPort, command("CH", CHANNELS))  
        #init_helper(serialPort, command("PORT", bytes(PORT_NUM, "UTF-8")))
        init_helper(serialPort, command("MODE", MODE))  
    except SerialTimeoutException as e:
        print("LoRa-E5: Could not issue command to module.", file=sys.stderr)

    return serialPort

'''Receive data over LoRa

   @param {Type[serial.Serial]) serialPort - serial port for LoRa module
 
   @return {str} received data
'''
def recv_LoRa(serialPort: Type[serial.Serial]) -> str:
    try:
        # read serial port
        received_data: str = serialPort.read_until(expected='', size=BUFF_SIZE).decode("UTF-8")
        sleep(0.03)

        # check for remaining byte
        data_left: int = serialPort.inWaiting()
        received_data += serialPort.read(data_left).decode("UTF-8")
    except SerialException as e:
        print("LoRa-E5: Could not receive response from module.", file=sys.stderr)

    return received_data


'''Send data over LoRa

   @param {Type[bytes]) nodeDate - data collected from active sensing modules
   @param {Type[serial.Serial]) serialPort - serial port for LoRa module
 
   @return {int} number of bytes written
'''
def send_LoRa(nodeData: Type[bytes], serialPort: Type[serial.Serial]) -> int:
    try:
        # send data
        sent_data: int = serialPort.write(command("MSG", nodeData))
        serialPort.flush()
    except SerialTimeoutException as e:
        print("LoRa-E5: Could not send data to module.", file=sys.stderr)

    return sent_data


'''Create data format string

   @param {list[str]} args - list of data collected from active sensing modules
 
   @return {str} data format string
'''
def create_data_format_str(args: list[str]) -> str:
    data_format_str: str = ""

    # append initial string argument format
    if len(args) >= 1:
        data_format_str += str(len(args[0])) + "s"

    # append additional string argument formats
    for i in range(1, len(args)):
        data_format_str += (" " + str(len(args[i])) + "s")

    return data_format_str


'''Pack data collected from active sensing modules

   @param {str) data_format - format string of how data will be packed
   @param {list[str]) args - list of data collected from active sensing modules
 
   @return {Type[bytes]} packed data in bytes
'''
def pack_data(data_format: str, args: list[str]) -> Type[bytes]:
    # tempRaw, tempC, tempF, hum, classification
    # create struct to hold data for LoRa transmission
    byteStream: Type[bytes] = struct.pack(data_format, *(bytes(args[i], "UTF-8") for i in range(len(args))))

    if DEBUG:
        print(struct.unpack(data_format, byteStream))

    # put converted data between quotations marks
    nodeData: Type[bytes] = bytes("\"", "UTF-8") + byteStream + bytes("\"", "UTF-8")

    if DEBUG:
        print(struct.unpack(data_format, byteStream))

    return nodeData


'''Wake up LoRa module

   @param {Type[serial.Serial]) serialPort - serial port for LoRa module
 
   @return {int} number of bytes written
'''
def awake_LoRa(serialPort: Type[serial.Serial]) -> int:
    try:
        sent_data: int = serialPort.write(b"\xff\xff\xff\xff" + command("LOWPOWER", bytes("AUTOOFF", "UTF-8")))
        serialPort.flush()
    except SerialTimeoutException as e:
        print("LoRa-E5: Could not exit deep sleep mode.", file=sys.stderr)

    return sent_data


'''Put LoRa module under low power mode
 
   @param {Type[serial.Serial]) serialPort - serial port for LoRa module

   @return {int} number of bytes written
'''
def sleep_LoRa(serialPort: Type[serial.Serial]) -> int:
    try:
        sent_data: int = serialPort.write(command("LOWPOWER", bytes("AUTOON", "UTF-8")))
        serialPort.flush()
    except SerialTimeoutException as e:
        print("LoRa-E5: Could not enter deep sleep mode.", file=sys.stderr)

    return sent_data