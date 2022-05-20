##
# File:     classify.py
# Author:   Diego Andrade (bets636@gmail.com)
# Brief:    Simple Imgage classification with mobilenet model
# Version:  0.1
# Date:     2022-05-20
# Note:     Taken from tflite python example
# 
# #

import tflite_runtime.interpreter as tflite

import numpy as np
from picamera import PiCamera
from PIL import Image

from io import BytesIO
from time import sleep

import time

# ---- Paths -------------------------------
TMP_PATH = '/home/NEST1/g3-osi-sw/test/ImageClassification/tmp'
MODEL_PATH = TMP_PATH + '/mobilenet_v1_1.0_224.tflite'
LABELS_PATH = TMP_PATH + '/labels.txt'

# ---- Default Parameters ---u---------------
input_mean = 127.5
input_std = 127.5
num_threads = None

def load_labels(filename):
  with open(filename, 'r') as f:
    return [line.strip() for line in f.readlines()]


if __name__ == '__main__':

    interpreter = tflite.Interpreter(
        model_path= MODEL_PATH,
        # experimental_delegates= ext_delegate,
        # num_threads= None
        )
    interpreter.allocate_tensors()

    input_details = interpreter.get_input_details()
    output_details = interpreter.get_output_details()

    # check the type of the input tensor
    floating_model = input_details[0]['dtype'] == np.float32

    # NxHxWxC, H:1, W:2
    height = input_details[0]['shape'][1]
    width = input_details[0]['shape'][2]

    # Get image
    stream = BytesIO()                          # Create the in-memory stream
    camera = PiCamera()
    camera.start_preview()
    sleep(2)
    camera.capture(stream, format='jpeg')
    stream.seek(0)                              # "Rewind" the stream to the beginning so we can read its content
    img = Image.open(stream).resize((width, height))
    #img = Image.open(args.image).resize((width, height))

    # add N dim
    input_data = np.expand_dims(img, axis=0)

    if floating_model:
        input_data = (np.float32(input_data) - input_mean) / input_std

    interpreter.set_tensor(input_details[0]['index'], input_data)

    start_time = time.time()
    interpreter.invoke()
    stop_time = time.time()

    output_data = interpreter.get_tensor(output_details[0]['index'])
    results = np.squeeze(output_data)

    top_k = results.argsort()[-5:][::-1]
    labels = load_labels(LABELS_PATH)
    for i in top_k:
        if floating_model:
            print('{:08.6f}: {}'.format(float(results[i]), labels[i]))
        else:
            print('{:08.6f}: {}'.format(float(results[i] / 255.0), labels[i]))

    print('time: {:.3f}ms'.format((stop_time - start_time) * 1000))
