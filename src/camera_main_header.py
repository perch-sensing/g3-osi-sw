import subprocess
import sys

# System calls and file names
CHECK_CAMERA_COM: str = "vcgencmd get_camera"
CAM_STAT_FLNM: str = "camera_status.txt"

def detectCamera() -> bool:
    subproc = subprocess.Popen(CHECK_CAMERA_COM, shell=True, stdout=subprocess.PIPE)
    subproc_return = subproc.stdout.read()
    string_ret = subproc_return.decode('UTF-8')
    if string_ret[-2] == '0':
        print("Camera: System did not detect a camera.", file=sys.stderr)
    return string_ret[-2] == '1'