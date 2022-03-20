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
BAND_PLAN = bytes("US915", "UTF-8")
DATA_RATE = bytes("DR3", "UTF-8")
CHANNELS = bytes("NUM,8-15", "UTF-8")
DEBUG: int = 1

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
LORA_MUX_SEL: tuple[int, int] = (1, 0)

COMM_HEADER: str = "AT"
COMM_END: str = "\r\n"
DEV_NAME: str = "/dev/ttyAMA0"

def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)

def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])

def init_helper(serialPort, comm_bytes) -> None:
    serialPort.write(comm_bytes)
    serialPort.flush()

    if DEBUG:
        print(recv_LoRa(serialPort))

def init_LoRa(dev_name: str):
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
    serialPort = serial.Serial(dev_name, BAUD_RATE, timeout=5)
    if DEBUG:
        print("Opened serial port")

    sleep(5)
    try:
        init_helper(serialPort, command("DR", BAND_PLAN))  
        init_helper(serialPort, command("DR", DATA_RATE))
        init_helper(serialPort, command("CH", CHANNELS))  
        init_helper(serialPort, command("PORT", bytes(PORT_NUM, "UTF-8")))
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not issue command to module.", file=sys.stderr)

    return serialPort

def command(comm, *args):
    fullCom = COMM_HEADER

    # append command
    if comm != "":
        fullCom = fullCom + "+" + comm

    # convert command into bytes object
    fullCom = bytes(fullCom, "UTF-8")

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

def recv_LoRa(serialPort):
    try:
        # read serial port
        received_data = serialPort.read_until(expected='', size=BUFF_SIZE).decode("UTF-8")
        sleep(0.03)

        # check for remaining byte
        data_left = serialPort.inWaiting()
        received_data += serialPort.read(data_left).decode("UTF-8")
    except serial.SerialException as e:
        print("LoRa-E5: Could not receive response from module.", file=sys.stderr)

    return received_data

def send_LoRa(nodeData, serialPort):
    try:
        # send data
        sent_data = serialPort.write(command("MSG", nodeData))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not send data to module.", file=sys.stderr)

    return sent_data

def pack_data(data_format: str, args: list[Any]):
    # tempRaw, tempC, tempF, hum, classification
    # create struct to hold data for LoRa transmission
    byteStream = struct.pack(data_format, *(bytes(args[i], "UTF-8") for i in range(len(args))))

    if DEBUG:
        print(struct.unpack(data_format, byteStream))

    # put converted data between quotations marks
    nodeData = bytes("\"", "UTF-8") + byteStream + bytes("\"", "UTF-8")

    if DEBUG:
        print(struct.unpack(data_format, byteStream))

    return nodeData

def awake_LoRa():
    try:
        sent_data = serialPort.write(b"\xff\xff\xff\xff" + command("LOWPOWER", bytes("AUTOOFF", "UTF-8")))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not exit deep sleep mode.", file=sys.stderr)

    return sent_data

def sleep_LoRa():
    try:
        sent_data = serialPort.write(command("LOWPOWER", bytes("AUTOON", "UTF-8")))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not enter deep sleep mode.", file=sys.stderr)

    return sent_data

def create_data_format_str(args: list[str]) -> str:
    data_format_str: str = ""

    # append initial string argument format
    if len(args) >= 1:
        data_format_str += str(len(args[0])) + "s"

    # append additional string argument formats
    for i in range(1, len(args)):
        data_format_str += (" " + str(len(args[i])) + "s")

    return data_format_str