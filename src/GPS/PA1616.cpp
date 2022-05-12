/**
 * @file PA1616.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief Implementation of PA1616.hpp module wrapper
 * @version 0.1
 * @date 2022-04-16
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "PA1616.hpp"

#include <sstream>
#include <unistd.h>

#include "../GPIOController.hpp"

PA1616::PA1616() {
    // Set up GPS pins
    GPIOController& gpio = GPIOController::getInstance();

    gpio.setMode(PA1616_SW_N_EN_PIN, OUTPUT);
    gpio.setMode(PA1616_NRESET_PIN, OUTPUT);

    gpio.setMode(PA1616_FIX_PIN,    INPUT);
    gpio.setPullUpDn(PA1616_FIX_PIN, PUD_OFF);

    // Default module state
    gpio.write(PA1616_SW_N_EN_PIN, HIGH);
    gpio.write(PA1616_NRESET_PIN, HIGH);
}

PA1616::~PA1616() {
    // In case left on
    powerOff();
}

PA1616& PA1616::getInstance()  {
    static PA1616 instance;     // Instantiated on first use.
    return instance;            // Guaranteed to be destroyed.
} 

void PA1616::powerOn() {
    GPIOController::getInstance().write(PA1616_SW_N_EN_PIN, LOW);
    sleep(1);
}

void PA1616::powerOff() {
    GPIOController::getInstance().write(PA1616_SW_N_EN_PIN, HIGH);
}

PA1616::RMC PA1616::getLocation() {
    return getLocation(100);
}

PA1616::RMC PA1616::getLocation(uint8_t tries) {
    RMC location;
    
    // Power GPS on
    powerOn();

    // Attempt to get fixed location for TRIES attemps
    GPIOController& gpio = GPIOController::getInstance();

    std::stringstream ss;
    std::string parsed_item;
    for ( ; tries > 0; tries--) {
        // Get GNGGA command
        do {
            // // Clear last command
            // ss.str(std::string());

            ss = gpio.serialReadLine();
            
            // Parse command type
            getline(ss, parsed_item, ',');
            
        } while (parsed_item.find("RMC") == std::string::npos);

        // Parse UTC time
        getline(ss, parsed_item, ',');
        location.UTC = parsed_item;

        // Parse status
        getline(ss, parsed_item, ',');
        location.STATUS = parsed_item;

        // Parse Latitude
        getline(ss, parsed_item, ',');
        location.LAT = parsed_item;

        // Parse N/S
        getline(ss, parsed_item, ',');
        location.N_S = parsed_item;

        // Parse Longitude
        getline(ss, parsed_item, ',');
        location.LON = parsed_item;

        // Parse E/W
        getline(ss, parsed_item, ',');
        location.E_W = parsed_item;

        // Check if fix found
        if (location.STATUS == "A") {
            return location; 
        }
    }

    // Power off to save power
    powerOff();

    location.STATUS = "V";
    return location;
}


