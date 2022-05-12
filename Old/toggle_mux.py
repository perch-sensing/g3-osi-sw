import RPi.GPIO as GPIO
MUX_SEL_A: int = 26
MUX_SEL_B: int = 32

def muxSelInit() -> None:
    GPIO.setup(MUX_SEL_A, GPIO.OUT)
    GPIO.setup(MUX_SEL_B, GPIO.OUT)

def setMuxSel(sel: tuple[int, int]) -> None:
    GPIO.output(MUX_SEL_B, sel[0])
    GPIO.output(MUX_SEL_A, sel[1])

GPIO.setwarnings(False)
GPIO.setmode(GPIO.BOARD)
muxSelInit()
setMuxSel((0,1))
#GPIO.cleanup()