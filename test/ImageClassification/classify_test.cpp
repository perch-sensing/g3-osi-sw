/**
 * @file classify_test.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief Simple single camera capture and classification with command line argument repeat
 * @version 0.1
 * @date 2022-05-20
 * @note Hardcoding python script location
 * 
 * @copyright Copyright (c) 2022
 * 
 */


#include <cstdlib>
#include <iostream>

#include "../../src/GPIOController.hpp"

#define FLAG_PIN 23

int main(int argc, char **argv) {
    // Setup
    GPIOController& gpio = GPIOController::getInstance();
    gpio.setMode(FLAG_PIN, OUTPUT);
    gpio.write(FLAG_PIN, LOW);

    uint8_t number_readings = 1;

    // Update number of readings from command line args
    if (argc > 1 && argv[1][0] >= '0' && argv[1][0] <= '9') {
        number_readings = std::stoi(argv[1]);
    }
    
    // Test
    for (; number_readings > 0; number_readings--) {
        gpio.write(FLAG_PIN, HIGH);
        system("python /home/NEST1/g3-osi-sw/src/ImageClassification/classify.py");
        gpio.write(FLAG_PIN, LOW);
    }
}

