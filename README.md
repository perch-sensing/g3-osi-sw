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
- This repo being located at home path, i.e. `~/`

Simple run
```shell
sh test/tflite_test.sh
```