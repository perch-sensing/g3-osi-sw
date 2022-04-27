/**
 * @file temperature_test.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief Simple single shot measurement reading of SHT30 on Perch G3 Compute Board
 * @version 0.1
 * @date 2022-04-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <iostream>

#include "../../src/Temperature/SHT30.hpp"

int main(int argc, char **argv) {
    uint8_t number_readings = 1;

    // Update number of readings from command line args
    if (argc > 1 && argv[1][0] >= '0' && argv[1][0] <= '9') {
        number_readings = std::stoi(argv[1]);
    }
    
    // Test
    SHT30& tempSensor = SHT30::getInstance();
    SHT30::Measurement reading;
    for (; number_readings > 0; number_readings--) {
        reading = tempSensor.getMeasurement();
        std::cout << "Temperature: " << reading.Temperature_C << " c" << std::endl; 
        std::cout << "Temperature: " << reading.Temperature_F << " F" << std::endl; 
        std::cout << "Temperature: " << reading.Humidity      << "% RH" << std::endl; 
        std::cout << std::endl;
    }
}
