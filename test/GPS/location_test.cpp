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

#include <iostream>

#include "../../src/GPS/PA1616.hpp"
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
    PA1616& gps = PA1616::getInstance();
    for ( ; number_readings > 0; number_readings--) {

        gpio.write(FLAG_PIN, HIGH);
        PA1616::RMC data = gps.getLocation();
        gpio.write(FLAG_PIN, LOW);

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
