
from time import sleep
from picamera import PiCamera
image_name = 'img.jpg'

/* Captures image and saves it into a file
 *
 * @return {str} name of image file
 */
def captureImage():
  camera = PiCamera()
  camera.resolution= (1024, 768)
  camera.start_preview()
  sleep(2)
  camera.capture(image_name)
  return image_name