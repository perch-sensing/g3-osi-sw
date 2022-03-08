import RPi.GPIO as GPIO
import serial
import struct
import sys
from time import sleep

LORA_RESET = 36
LORA_BOOT = 38
BUFF_SIZE = 500
DATA_RATE = "DR3"
CHANNELS = "NUM,8-15"
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
        sent_data = serialPort.write(command("MODE", "TEST").encode())
        serialPort.flush()
        sent_data = serialPort.write(command("DR", DATA_RATE).encode())
        serialPort.flush()
        sent_data = serialPort.write(command("CH", CHANNELS).encode())
        serialPort.flush()
    except serialPort.SerialTimeoutException as e:
        print("LoRa-E5: Could not issue command to module.", file=sys.stderr)
    return serialPort

def command(comm, *args):
    fullCom = "AT"
    if comm != "":
        fullCom = fullCom + "+" + comm
    if len(args) > 0:
        fullCom = fullCom + "=" + str(args[0])
        for i in range(1, len(args)):
            fullCom = fullCom + " " + str(args[i])
            fullCom += "\r\n"
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
    except serialPort.SerialException as e:
        print("LoRa-E5: Could not receive response from module.", file=sys.stderr)
    return received_data

def send_LoRa(nodeData, serialPort):
    try:
        # send data
        sent_data = serialPort.write(command('CMSG', nodeData).encode())
        serialPort.flush()
    except serialPort.SerialTimeoutException as e:
        print("LoRa-E5: Could not send data to module.", file=sys.stderr)
    return sent_data

# CHECK ENDIANNESS
def pack_data(*args):
    # tempRaw, tempC, tempF, hum, classification
    # create struct to hold data for LoRa transmission
    byteStream = struct.pack('i f f f' + ' 15s'*len(classification), args[0], args[1], args[2], args[3], *(bytes(elem, 'UTF-8') for elem in args[4]))
    nodeData = str(byteStream)
    nodeData = "\"" + nodeData + "\""
    return nodeData

def awake_LoRa():
    try:
        sent_data = serialPort.write(b'\xff\xff\xff\xff' + command('LOWPOWER', "AUTOOFF").encode())
        serialPort.flush()
    except serialPort.SerialTimeoutException as e:
        print("LoRa-E5: Could not exit deep sleep mode.", file=sys.stderr)
    return sent_data

def sleep_LoRa():
    try:
        sent_data = serialPort.write(command('LOWPOWER', "AUTOON").encode())
        serialPort.flush()
    except serialPort.SerialTimeoutException as e:
        print("LoRa-E5: Could not enter deep sleep mode.", file=sys.stderr)
    return sent_data

GPIO.cleanup()
GPIO.setmode(GPIO.BOARD)
muxSelInit()
setMuxSel(LORA_MUX_SEL)
serialPort = init_LoRa()
sleep(5)
sent_data = serialPort.write(command("JOIN").encode())
respon = recv_LoRa(serialPort)
print(respon)