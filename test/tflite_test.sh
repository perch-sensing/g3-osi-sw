BASE_DIR=~/g3-osi-sw
TEST_DIR=${BASE_DIR}/test

PHOTO_FILE=${TEST_DIR}/tmp/grace_hopper.bmp
MODEL_FILE=${TEST_DIR}/tmp/mobilenet_v1_1.0_224.tflite
LABELS_FILE=${TEST_DIR}/tmp/labels.txt

# Set up tmp directory
if ! test -e "${TEST_DIR}/tmp"; then
    echo "\nDownloading needed files to ${TEST_DIR}/tmp directory..."
    mkdir ${TEST_DIR}/tmp
fi

# Set up test photo 
if ! test -f "$PHOTO_FILE"; then
    echo "\nDownloading example image..."
    curl https://raw.githubusercontent.com/tensorflow/tensorflow/master/tensorflow/lite/examples/label_image/testdata/grace_hopper.bmp > $PHOTO_FILE
fi

# Set up model mobilenet_v1__1.0_224
if ! test -f "$MODEL_FILE"; then
    # Get model
    echo "\nDownloading tflite model..."
    curl https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_2018_02_22/mobilenet_v1_1.0_224.tgz | tar xzv -C ${TEST_DIR}/tmp
fi

# Set up labels.txt
if ! test -f "$LABELS_FILE"; then
    echo "\nDownloading label file..."
    curl https://storage.googleapis.com/download.tensorflow.org/models/mobilenet_v1_1.0_224_frozen.tgz  | tar xzv -C ${TEST_DIR}/tmp  mobilenet_v1_1.0_224/labels.txt
    mv ${TEST_DIR}/tmp/mobilenet_v1_1.0_224/labels.txt ${TEST_DIR}/tmp
fi

# Run test
echo "\nRunning classification on example image $PHOTO_FILE:"
python3 ${TEST_DIR}/label_image.py \
--model_file $MODEL_FILE \
--label_file $LABELS_FILE \
--image $PHOTO_FILE
