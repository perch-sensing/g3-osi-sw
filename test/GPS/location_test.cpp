/**
 * @file PA1616_Test.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdint>
#include <iostream>
#include <wiringPi.h>

#include "../../src/GPS/PA1616.hpp"
#include "../../src/GPIOController.hpp"

#define MUX_A_PIN 26
#define MUX_B_PIN 32

int main(int argc, char **argv) {
    uint8_t number_readings = 1;

    // Update number of readings from command line args
    if (argc > 1 && argv[1][0] >= '0' && argv[1][0] <= '9') {
        number_readings = std::stoi(argv[1]);
    }

    // Setup UART mux and select GPS
    GPIOController& gpio = GPIOController::getInstance();
    gpio.setMode(MUX_A_PIN, OUTPUT);
    gpio.setMode(MUX_B_PIN, OUTPUT);

    gpio.write(MUX_A_PIN, HIGH);
    gpio.write(MUX_B_PIN, LOW);

    // Test
    PA1616& gps = PA1616::getInstance();
    for ( ; number_readings > 0; number_readings--) {
        PA1616::RMC data = gps.getLocation();

        std::cout << data.UTC << std::endl; 
        std::cout << data.STATUS << std::endl; 
        std::cout << data.LAT << std::endl; 
        std::cout << data.N_S << std::endl; 
        std::cout << data.LON << std::endl; 
        std::cout << data.E_W << std::endl; 

        std::cout << std::endl;
    }

    return 0;
}
