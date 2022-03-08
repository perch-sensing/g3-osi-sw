import sys
import sht30
from time import sleep

from typing import *

TH_NUM_FIELDS: int = 3
DEBUG: int = 1

def main() -> None:
  stderr_fileno: int = sys.stderr
  sys.stderr = open("errors.log", "w")

  fd: int = sht30.initialize("/dev/i2c-1")
  cnt: int = 0

  if (fd >= 0):
    temp: list[int] = [0 for i in range(2)]
    temp_hum_arr: list[float] = [0.0 for i in range(TH_NUM_FIELDS)]
    while cnt < 50:
      if (sht30.processData(fd, temp, temp_hum_arr) >= 0):
        if (DEBUG):
          print("\nTemperature:", temp[0])
          print("Temperature (C):", temp_hum_arr[0])
          print("Temperature (F):", temp_hum_arr[1])
          print("Humidity:", temp_hum_arr[2])
      sleep(2)
      cnt += 1
  sys.stderr.close()
  sys.stderr = stderr_fileno

main()
