import sys
import sht30

from typing import *

TH_NUM_FIELDS: int = 3
DEBUG = 1

def main() -> None:
  stderr_fileno: int = sys.stderr
  sys.stderr = open("errors.log", "w")

  fd: int = sht30.initialize()

  if (fd >= 0):
    temp: int = 0
    temp_hum_arr: list[float] = [0.0 for i in range(TH_NUM_FIELDS)]

    if (sht30.processData(fd, temp, temp_hum_arr) >= 0):
      if (DEBUG):
        print("\nTemperature:", temp)
        print("Temperature (C):", temp_hum_arr[0])
        print("Temperature (F):", temp_hum_arr[1])
        print("Humidity:", temp_hum_arr[2])
  
  sys.stderr.close()
  sys.stderr = stderr_fileno

main()