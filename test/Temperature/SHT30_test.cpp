/**
 * @file SHT30_test.cpp
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

int main() {
    SHT30& tempSensor = SHT30::getInstance();

    SHT30::Measurement reading = tempSensor.getMeasurement();
    std::cout << "Temperature: " << reading.Temperature_C << " c" << std::endl; 
    std::cout << "Temperature: " << reading.Temperature_F << " F" << std::endl; 
    std::cout << "Temperature: " << reading.Humidity << "% RH" << std::endl; 
}
