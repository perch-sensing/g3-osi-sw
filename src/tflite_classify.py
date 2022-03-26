import numpy as np
import time
import os
from tflite_runtime.interpreter import Interpreter 
from PIL import Image
from typing import *

DEBUG: int = 1
TOP_K: int = 5
DATA_FOLDER: str = "/home/pi/MobileNet_v2/"
MODEL_FILE: str = "mobilenet_v2_1.0_224_quantized_1_metadata_1.tflite"
LABEL_FILE: str = "labels.txt"

'''Creates a list of labels extracted from a given file
 
   @param {str} path - file name of labels file
 
   @return {list[str]} List of label strings
'''
def load_labels(path: str) -> list[str]:
  # Read the labels from the text file as a Python list.
  with open(path, 'r') as f:
    return [line.strip() for i, line in enumerate(f.readlines())]


'''Assign image data to the input tensor of the model

   @param {Type[Interpreter]} interpreter - interpreter interface for tflite models
   @param {Type[Image]}  image - PIL.Image object of image
  
   @return {void}
'''
def set_input_tensor(interpreter: Type[Interpreter], image: Type[Image]) -> None:
  tensor_index = interpreter.get_input_details()[0]['index']
  input_tensor = interpreter.tensor(tensor_index)()[0]
  input_tensor[:, :] = image


'''Run image through object detection model for classification list
 
   @param {Type[Interpreter]} interpreter - interpreter interface for tflite models
   @param {Type[Image]} image - PIL.Image object of image
   @param {int} top_k - number of top entries of classification list
  
   @return {int} Identification number for label index within labels list
   @return {float} Probability of object being accurately classified
'''
def classify_image(interpreter: Type[interpreter], image: Type[Image], top_k: int) -> tuple[list[int], list[float]]:
  # Set image as input tensor
  set_input_tensor(interpreter, image)

  # Process image through tflite model and get output details
  interpreter.invoke()
  output_details = interpreter.get_output_details()[0]
  output = np.squeeze(interpreter.get_tensor(output_details['index']))
  output = output.astype('int8')

  scale, zero_point = output_details['quantization']
  output = scale * (output - zero_point)

  # Softmax Activation Function
  exponentials = np.exp(output)
  sum_exponentials = sum(exponentials)
  output = exponentials/sum_exponentials

  # Obtain the ordered classification list based on highest confidence values
  ordered = np.argsort(-output)
  label_id = []
  prob = []
  for i in ordered[:top_k]:
    label_id.append(i)
    prob.append(output[i])
  return label_id, prob


'''Load object detection model
 
   @return {Type[Interpreter]} Interpreter interface for tflite models
   @return {list[str]} Labels list for object classification
   @return {int} Width of input tensor
   @return {int) Height of input tensor
'''
def load_model() -> tuple[Type[Interpreter], list[str], int, int]:
  data_folder = DATA_FOLDER

  model_path = data_folder + MODEL_FILE
  label_path = data_folder + LABEL_FILE

  interpreter = Interpreter(model_path)
  if DEBUG:
    print("Model Loaded Successfully.")

  interpreter.allocate_tensors()
  _, height, width, _ = interpreter.get_input_details()[0]['shape']
  if DEBUG:
    print("Image Shape (", width, ",", height, ")")
 
  # Read class labels.
  labels = load_labels(label_path)

  return interpreter, labels, width, height


'''Process image through object detection model
 
   @param {str} img_name - file name of image
   @param {Type[Interpreter]} interpreter - interpreter interface for tflite models
   @param {list[str]} labels - labels list for object classification
   @param {int} width - width of input tensor
   @param {int) height - height of input tensor
   
   @return {list[str]} Labels list of detected objects in image
'''
def process_image(img_name: str, interpreter: Type[Interpreter], labels: list[str], width: int, height: int) -> list[str]:
  # Load an image to be classified. Convert RGB-ordered pixel data into BGR-order.
  image = Image.open("./" + img_name).convert('RGB').resize((width, height))
  container = list(image.getdata())
  wid, hgt = image.size
  for i in range(wid*hgt):
    container[i] = container[i][::-1]
  image.putdata(container)

  # Classify the image.
  time1 = time.time()
  label_id, prob = classify_image(interpreter, image, TOP_K)
  time2 = time.time()
  classification_time = np.round(time2-time1, 3)
  if DEBUG == 1:
    print("Classification Time =", classification_time, "seconds.")

  img_labels = []

  # Return the classification label of the image.
  for ind in range(len(label_id)):
    classification_label = labels[label_id[ind]]
    if DEBUG == 1:
      print("Image Label is:", classification_label, ", with Likeliness:", np.round(prob[ind]*100, 2), "%.")
    img_labels.append(classification_label)
  os.remove(img_name)
  
  return img_labels