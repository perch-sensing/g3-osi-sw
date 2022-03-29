from typing import *
import RPi.GPIO as GPIO

MUX_SEL_A: int = 26
MUX_SEL_B: int = 32
LOAD_MUX_SEL: tuple[int, int] = (0, 0)
GPS_MUX_SEL:  tuple[int, int] = (0, 1)
LORA_MUX_SEL: tuple[int, int] = (1, 0)
CPPC_MUX_SEL: tuple[int, int] = (1, 1)


'''Initialize mux select
 
   @return {None}	
'''
def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)


'''Set mux select

   @param {tuple[int, int]} sel - mux select configuration
 
   @return {None}	
'''
def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])