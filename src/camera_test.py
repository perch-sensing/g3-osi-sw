
from rpi_camera import *
from tflite_classify import *

/* Pipeline for capturing image, processing image through object detection model, and 
 * returning labels list of detected objects in image
 *
 * @return {list[str]} Labels list of detected objects in image
 */
def classification_pipeline():
  interpreter, labels, width, height = load_model()
  label_list = process_image(captureImage(), interpreter, labels, width, height)
  if debug == 1:
    for label in  label_list:
      print(label)
  return label_list

classification_pipeline()