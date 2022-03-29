from typing import *

ERRORS_LOG_NAME: str = "errors.log"
GPS_ERRORS_LOG_NAME: str = "gps-errors.log"
UART_PORT: str = "/dev/ttyAMA0"

# Bit masks
GPS_VALID_MASK: int = 0b1000
TH_VALID_MASK: int = 0b0100
CAMERA_VALID_MASK: int = 0b0010
LORA_VALID_MASK: int = 0b0001

def clear_valid_flag(valid_flag_mask: int) -> None:
    valid_flags &= ~valid_flag_mask

def check_valid_flag(valid_flag_mask: int) -> bool:
    return valid_flags & valid_flag_mask