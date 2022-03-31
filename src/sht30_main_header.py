from typing import *
import sht30

# Temperature/Humidity literals
TH_DATA_SIZE: int = 6
TH_NUM_FIELDS: int = 3
TH_BUS: str = "/dev/i2c-1"

def TH_Test(TH_File: int, TH_buffer: list[int]) -> bool:
    return sht30.readData(TH_File, TH_buffer) >= 0