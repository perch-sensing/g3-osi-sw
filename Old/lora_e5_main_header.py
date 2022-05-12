from typing import *
from lora_e5 import *

def test_LoRa(LoRa_File: Type[serial.Serial]) -> bool:
    LoRa_File.write(command(""))
    return recv_LoRa(LoRa_File) == "+AT: OK\r\n"
