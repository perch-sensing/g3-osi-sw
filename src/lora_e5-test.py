from lora_e5 import *

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
muxSelInit()
setMuxSel(LORA_MUX_SEL)
serialPort = init_LoRa(DEV_NAME)
sleep(5)

if DEBUG:
    serialPort.write(command(""))
    print(recv_LoRa(serialPort))

    serialPort.write(command("MSG"))
    print(recv_LoRa(serialPort))

'''
# obtain DevEui and DevAddr and set NWKSKEY and APPSKEY
serialPort.write(command("ID",bytes("DevEui", "UTF-8")))
print(recv_LoRa(serialPort))
serialPort.write(command("ID",bytes("DevAddr", "UTF-8")))
print(recv_LoRa(serialPort))
serialPort.write(command("KEY",bytes("NWKSKEY", "UTF-8"),bytes("50726F6A6563744E6573745065726368", "UTF-8")))
print(recv_LoRa(serialPort))
serialPort.write(command("KEY",bytes("APPSKEY", "UTF-8"),bytes("506572636850726F6A6563744E657374", "UTF-8")))
print(recv_LoRa(serialPort))
'''
args = [str(20), str(45.7), str(38.2), str(97.1), "fire", "", "", ""]

if DEBUG:
    print(create_data_format_str(args))

nodeData = pack_data(create_data_format_str(args), args)
send_LoRa(nodeData, serialPort)

if DEBUG:
    print(recv_LoRa(serialPort))


serialPort.close()
GPIO.cleanup()