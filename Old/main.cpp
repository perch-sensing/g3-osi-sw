#include "main.hpp"

int8_t detectCamera() {
    if (system(CHECK_CAMERA_COM) < 0) {
        return -1;
    }
    ifstream camStat(CAM_STAT_FLNM);
    //***parse camStat file
    //***return -1 if no camera detected, 0 if detected
}

int8_t tarFile(const char* command) {
    if (system(command) < 0) {
        return -1;
    }
    return 0;
}

/*
flags -> count number of flags needed, choose appropriate variable size, create header file with bit mask macros
how to run an update for an application
how to enable GPIO pin
is on-board temperature sensor available for use? -> reference for SHT-30 test
*/

ofstream errorLog("errors.log");
ofstream GPSErrorLog("gps-errors.log");

int main() {
    uint8_t stage = 0;

    // File descriptors
    int32_t GPS_File, TH_File;

    // Data containers
    uint8_t TH_buffer[TH_DATA_SIZE];
    float TH_Data[TH_NUM_FIELDS];
    char GPS_Data[GPS_MSG_SIZE];
    struct GPSPkg GPS_Pkg;  

    cerr.rdbuf(errorLog.rdbuf());
    if (detectCamera()) {
        cerr << "Camera: System did not detect a camera" << endl;
        // ***set flag for "camera access"
    }

    // Not decided yet - switch mux to "Load program" input

    // Force GPS enable to low (GPIO6, pin 31)

    if (stage == 0) {
        
        // ***GPS enable set to high

        cerr.rdbuf(GPSErrorLog.rdbuf());
        if ((GPS_File = OpenGPSPort(UART_DEV)) == -1) {
            // ***clear flag for "gps valid"
        }

        cerr.rdbuf(errorLog.rdbuf());
        if ((TH_File = initialize()) == -1) {
            // ***clear flag for "th valid"
        }
        else {
            if (readData(TH_File, &TH_buffer) < 0) {
                // ***clear flag for "th valid"
                // ***if on-board sensor is available, compare values
            }
        }

        // ***Configure gas sensor

        // ***Run gas sensor test

        // ***Run camera test -> capture image and see if image file is present

        stage = 1;
    }

    if (stage == 1) {
        cerr.rdbuf(GPSErrorLog.rdbuf());
        if (obtainFix(GPS_File, &GPS_Data) < 0) {
            // ***clear flag for "gps valid"
            stage = 3;
        }
        else
            stage = 2;
        
        //***tar up gps-errors.log
        //***send tar-ed up file over LoRa

        cerr.rdbuf(errorLog.rdbuf());
    }

    if (stage == 2) {
        packageGPSData(GPS_Data, &GPS_Pkg);

        //***send packaged GPS data over LoRa

        stage = 3;
    }

    if (stage == 3) {
        while (1) {
            //***take picture
            //***post-process with model

            if (processData(file, &temp, &TH_Data) < 0) {
                // ***clear flag for "th valid"
            }

            //***read gas data and process it

            //***package data into struct

            if (!tarFile(TAR_ERR_FILE_COM)) {
                //***send errors.tar.gz over LoRa
            }
            else {
                //***send errors-dummy.tar.gz over LoRa
            }

            //***send packaged data over LoRa

            //***deep sleep mode for 2 minutes
        }
    }

    errorLog.close();
    GPSErrorLog.close();
    return 0;
}