
from tflite_runtime.interpreter import Interpreter 
from PIL import Image
import numpy as np
import time
import os

debug = 1

/* Creates a list of labels extracted from a given file
 *
 * @param {str} path - file name of labels file
 *
 * @return {list[str]} List of label strings
 */
def load_labels(path): # Read the labels from the text file as a Python list.
  with open(path, 'r') as f:
    return [line.strip() for i, line in enumerate(f.readlines())]

/* Assign image data to the input tensor of the model
 *
 * @param {tflite_runtime.interpreter.Interpreter} interpreter - interpreter interface for tflite models
 * @param {PIL.Image}  image - PIL.Image object of image
 * 
 * @return {void}
 */
def set_input_tensor(interpreter, image):
  tensor_index = interpreter.get_input_details()[0]['index']
  input_tensor = interpreter.tensor(tensor_index)()[0]
  input_tensor[:, :] = image

/* Run image through object detection model for classification list
 *
 * @param {tflite_runtime.Interpreter} interpreter - interpreter interface for tflite models
 * @param {PIL.Image} image - PIL.Image object of image
 * @param {int32} top_k - number of top entries of classification list
 * 
 * @return {int32} Identification number for label index within labels list
 * @return {float} Probability of object being accurately classified
 */
def classify_image(interpreter, image, top_k=5):
  set_input_tensor(interpreter, image)

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

  ordered = np.argsort(-output)
  label_id = []
  prob = []
  for i in ordered[:top_k]:
    label_id.append(i)
    prob.append(output[i])
  return label_id, prob

/* Load object detection model
 *
 * @return {tflite_runtime.Interpreter} Interpreter interface for tflite models
 * @return {list[str]} Labels list for object classification
 * @return {int32} Width of input tensor
 * @return {int32) Height of input tensor
 */
def load_model():

  data_folder = "/home/pi/MobileNet_v2/"

  model_path = data_folder + "mobilenet_v2_1.0_224_quantized_1_metadata_1.tflite"
  label_path = data_folder + "labels.txt"

  interpreter = Interpreter(model_path)
  if debug == 1:
    print("Model Loaded Successfully.")

  interpreter.allocate_tensors()
  _, height, width, _ = interpreter.get_input_details()[0]['shape']
  if debug == 1:
    print("Image Shape (", width, ",", height, ")")
 
  # Read class labels.
  labels = load_labels(label_path)

  return interpreter, labels, width, height

/* Process image through object detection model
 *
 * @param {str} img_name - file name of image
 * @param {tflite_runtime.Interpreter} interpreter - interpreter interface for tflite models
 * @param {list[str]} labels - labels list for object classification
 * @param {int32} width - width of input tensor
 * @param {int32) height - height of input tensor
 * 
 * @return {list[str]} Labels list of detected objects in image
 */
def process_image(img_name, interpreter, labels, width, height):

  # Load an image to be classified. Convert RGB-ordered pixel data into BGR-order.
  image = Image.open("./" + img_name).convert('RGB').resize((width, height))
  container = list(image.getdata())
  wid, hgt = image.size
  for i in range(wid*hgt):
    container[i] = container[i][::-1]
  image.putdata(container)

  # Classify the image.
  time1 = time.time()
  label_id, prob = classify_image(interpreter, image)
  time2 = time.time()
  classification_time = np.round(time2-time1, 3)
  if debug == 1:
    print("Classification Time =", classification_time, "seconds.")

  img_labels = []

  # Return the classification label of the image.
  for ind in range(len(label_id)):
    classification_label = labels[label_id[ind]]
    if debug == 1:
      print("Image Label is:", classification_label, ", with Likeliness:", np.round(prob[ind]*100, 2), "%.")
    img_labels.append(classification_label)
  os.remove(img_name)
  
  return img_labels