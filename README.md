# Perch 3GRMC SW

## SW

TBD

## Diagrams

The diagrams are created with [PlantUML](https://plantuml.com/). To install this
plugin, type <kbd>Ctrl</kbd> + <kbd>P</kbd>. Then, enter `ext install plantuml`.

To enable rendering of PlantUML diagrams, you'll need to install the PlantUML
server. This is fairly trivial, and can be accomplished with the following
command (you'll need to have Docker installed):

```shell
docker run -d -p 8187:8080 --name plantml plantuml/plantuml-server:jetty
```

You also need to edit your `settings.json` file and add the following lines:

```json
"plantuml.server": "http://localhost:8187",
"plantuml.render": "PlantUMLServer",
```

If you changed the port or the host of the PlantUML server, edit
`plantuml.server` appropriately.

To open `settings.json`, type <kbd>Ctrl</kbd> + <kbd>P</kbd>, then enter:

```text
>Preferences: Open Settings (JSON)
```

## Tensorflow Lite 

### Setup
Tensorflow Lite is not compiled for Raspberry Pi Zero, as such, it has been included under the ```Wheels``` directory. The following steps walk you through the installation.

Required python version: >=3.9.2 

1. (Recommended) Create virtual enviornment to help isolate project specific dependencies
    1. Create virtual enviornment
        ```shell 
        python -m venv .venv            # or other name
        ```
    2. Activate virtual enviornment
        ```shell
        source .venv/bin/activate       # or other name
        ```
2. Install python modules
    ```shell
    pip install -r requirements.txt 
    ```
3. Install libraries
    ```shell
    sudo apt-get install libatlas-base-dev libopenjp2-7
    ```

### Testing
Testing of tflite-runtime can be done with `tflite_test` script located in test folder.

Requirements:
- Internet connectivity: For downloading example files

Simply run:
```shell
sh test/ImageClassification/tflite_test.sh
```

# Sensor Modules
## GPIO 
All modules use the [WiringPi C](https://github.com/WiringPi/WiringPi) library, which cames pre-installed on Raspberry OS install. If not, it will need to be compiled and installed natively following the setup instructions below.

### Setup
1. Clone [WiringPi](https://github.com/WiringPi/WiringPi) repo
    ```shell
    git clone https://github.com/WiringPi/WiringPi.git
    ```
2. Navigate to build script
    ```shell
    cd WiringPi
    ```
3. Run build script
    ```shell
    ./build
    ```

WiringPi should now be installed. The WiringPi source code is no longer needed and can be removed.

### Testing 
WiringPi install comes with various cli tools, one of which is [gpio](http://wiringpi.com/the-gpio-utility/) which can be used to test successful install. More info can be found at http://wiringpi.com/the-gpio-utility

```shell
gpio -v
```

Which should respond with a message similar to:
```shell
gpio version: 2.70
Copyright (c) 2012-2018 Gordon Henderson
...
...
```

## Modules

Each module has a corresponding sample test. Building all test can be done by running the following command

```shell
make Test
```

Additional each module has it's own `make` command. They consist of:

```shell
make Temperature_Test
make GPS_Test
make LoRa_Test
```

The follow test will them be made available:

```shell
./build/test/temperature_test <N>   ; Defaults to 1 if no N given
./build/test/location_test <N>      ; Defaults to 1 if no N given
./build/test/lora_test <N> <Data>   ; Defaults to 1 if no N given, data defaults to `TEST`
```

