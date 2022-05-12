from lora_e5 import *
import serial
from mux import *

DEV_NAME: str = "/dev/ttyAMA0"

def main():
    GPIO.setwarnings(False)
    GPIO.setmode(GPIO.BOARD)
    
    muxSelInit()
    setMuxSel(LORA_MUX_SEL)
    serialPort = init_LoRa(DEV_NAME)
    sleep(5)
    
    if DEBUG:
        serialPort.write(command(""))
        recvBuff: str = recv_LoRa(serialPort)
        print(recvBuff)

        #serialPort.write(command("MSG"))
        #print(recv_LoRa(serialPort))

    # obtain DevEui and DevAddr and set NWKSKEY and APPSKEY
    '''
    serialPort.write(command("ID",bytes("DevEui", "UTF-8")))
    print(recv_LoRa(serialPort))
    serialPort.write(command("ID",bytes("DevAddr", "UTF-8")))
    print(recv_LoRa(serialPort))
    serialPort.write(command("KEY",bytes("NWKSKEY", "UTF-8"),bytes("50726F6A6563744E6573745065726368", "UTF-8")))
    print(recv_LoRa(serialPort))
    serialPort.write(command("KEY",bytes("APPSKEY", "UTF-8"),bytes("506572636850726F6A6563744E657374", "UTF-8")))
    print(recv_LoRa(serialPort))
    
    serialPort.write(command("KEY",bytes("APPKEY", "UTF-8"),bytes("\"A4F8B352ECE031C95D0F8762A3FDF953\"", "UTF-8")))
    print(recv_LoRa(serialPort))
    
    serialPort.write(command("ID",bytes("AppEui", "UTF-8")))
    print(recv_LoRa(serialPort))
    '''
    args = [str(20), str(45.7), str(38.2), str(97.1), "fire", "", "", ""]
    
    nodeData = pack_data(args)
    
    send_LoRa(nodeData, serialPort)
    
    if DEBUG:
        print(recv_LoRa(serialPort))
    
    serialPort.write(command("JOIN"))

    if DEBUG:
        print(recv_LoRa(serialPort))

    sleep_LoRa(serialPort)
    sleep(10)
    awake_LoRa(serialPort)
    if DEBUG:
        serialPort.write(command(""))
        recvBuff: str = recv_LoRa(serialPort)
        print(recvBuff)
    
    serialPort.close()
    
    GPIO.cleanup()

main()