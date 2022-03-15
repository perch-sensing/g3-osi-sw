import RPi.GPIO as GPIO
import serial
import struct
import sys
from typing import *
from time import sleep

LORA_RESET = 36
LORA_BOOT = 38
BUFF_SIZE = 500
BAND_PLAN = bytes("US915", "UTF-8")
DATA_RATE = bytes("DR3", "UTF-8")
CHANNELS = bytes("NUM,8-15", "UTF-8")
DEBUG = 1

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
LORA_MUX_SEL: tuple[int, int] = (1, 0)

def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)

def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])


def init_LoRa():
    GPIO.setup(LORA_RESET, GPIO.OUT) 
    GPIO.setup(LORA_BOOT, GPIO.OUT)
    # shift LoRa into AT Application mode
    GPIO.output(LORA_BOOT, GPIO.HIGH)
    sleep(1)
    # reset the LoRa (reset is active low)
    GPIO.output(LORA_RESET, GPIO.LOW)
    sleep(5)
    GPIO.output(LORA_RESET, GPIO.HIGH)
    serialPort = serial.Serial("/dev/ttyAMA0", 9600, timeout=5)
    if DEBUG:
        print("Opened serial port")
    try:
        sent_data = serialPort.write(command("MODE", bytes("TEST", "UTF-8")))
        serialPort.flush()
        print(recv_LoRa(serialPort))
        sent_data = serialPort.write(command("DR", BAND_PLAN))
        serialPort.flush()
        print(recv_LoRa(serialPort))
        sent_data = serialPort.write(command("DR", DATA_RATE))
        serialPort.flush()
        print(recv_LoRa(serialPort))
        sent_data = serialPort.write(command("CH", CHANNELS))
        serialPort.flush()
        print(recv_LoRa(serialPort))
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not issue command to module.", file=sys.stderr)
    return serialPort

def command(comm, *args):
    fullCom = "AT"
    if comm != "":
        fullCom = fullCom + "+" + comm
    fullCom = bytes(fullCom, "UTF-8")
    if len(args) > 0:
        fullCom = fullCom + bytes("=", "UTF-8") + args[0]
        for i in range(1, len(args)):
            fullCom = fullCom + bytes(" ", "UTF-8") + args[i]
    fullCom += bytes("\r\n", "UTF-8")
    print(fullCom)
    return fullCom

def recv_LoRa(serialPort):
    try:
        # read serial port
        received_data = serialPort.read_until(expected='', size=BUFF_SIZE).decode('UTF-8')
        sleep(0.03)
        # check for remaining byte
        data_left = serialPort.inWaiting()
        received_data += serialPort.read(data_left).decode('UTF-8')
    except serial.SerialException as e:
        print("LoRa-E5: Could not receive response from module.", file=sys.stderr)
    return received_data

def send_LoRa(nodeData, serialPort):
    try:
        # send data
        sent_data = serialPort.write(command('MSG', nodeData))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not send data to module.", file=sys.stderr)
    return sent_data

# CHECK ENDIANNESS
def pack_data(data_format: str, args_str_bound: int, args: list[Any]):
    # tempRaw, tempC, tempF, hum, classification
    # create struct to hold data for LoRa transmission
    byteStream = struct.pack(data_format, *(args[i] for i in range(args_str_bound)), *(bytes(args[i], 'UTF-8') for i in range(args_str_bound, len(args))))
    nodeData = bytes('\"', 'UTF-8') + byteStream + bytes('\"', 'UTF-8')
    return nodeData

def awake_LoRa():
    try:
        sent_data = serialPort.write(b'\xff\xff\xff\xff' + command('LOWPOWER',bytes("AUTOOFF", "UTF-8")))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not exit deep sleep mode.", file=sys.stderr)
    return sent_data

def sleep_LoRa():
    try:
        sent_data = serialPort.write(command('LOWPOWER', bytes("AUTOON", "UTF-8")))
        serialPort.flush()
    except serial.SerialTimeoutException as e:
        print("LoRa-E5: Could not enter deep sleep mode.", file=sys.stderr)
    return sent_data

GPIO.setwarnings(False)
GPIO.cleanup()
GPIO.setmode(GPIO.BOARD)
muxSelInit()
setMuxSel(LORA_MUX_SEL)
serialPort = init_LoRa()
sleep(5)
sent_data = serialPort.write(command(""))
print(recv_LoRa(serialPort))
'''
serialPort.write(command("ID",bytes("DevEui", "UTF-8")))
print(recv_LoRa(serialPort))
serialPort.write(command("ID",bytes("AppEui", "UTF-8")))
print(recv_LoRa(serialPort))
serialPort.write(command("KEY",bytes("APPKEY", "UTF-8"),bytes("A4F8B352ECE031C95D0F8762A3FDF953", "UTF-8")))
print(recv_LoRa(serialPort))
'''
nodeData = pack_data("i f f f 15s 15s 15s 15s", 4, [20, 45.7, 38.2, 97.1, "fire", "", "", ""])
sent_data = send_LoRa(nodeData, serialPort)
print(recv_LoRa(serialPort))
