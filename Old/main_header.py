from typing import *

ERRORS_LOG_NAME: str = "errors.log"
GPS_ERRORS_LOG_NAME: str = "gps-errors.log"
UART_PORT: str = "/dev/ttyAMA0"

# Global variable for flags
valid_flags: int = 0b1111 # L->R: GPS, TH, Camera, LoRa

# Bit masks
GPS_VALID_MASK: int = 0b1000
TH_VALID_MASK: int = 0b0100
CAMERA_VALID_MASK: int = 0b0010
LORA_VALID_MASK: int = 0b0001

def clear_valid_flag(valid_flag_mask: int) -> None:
    global valid_flags
    valid_flags &= ~valid_flag_mask

def set_valid_flag(valid_flag_mask: int) -> None:
    global valid_flags
    valid_flags |= valid_flag_mask

def check_valid_flag(valid_flag_mask: int) -> bool:
    global valid_flags
    return valid_flags & valid_flag_mask