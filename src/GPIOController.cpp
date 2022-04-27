/**
 * @file GPIOController.cpp
 * @author your name (you@domain.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "GPIOController.hpp"

#include <wiringPi.h>

GPIOController::GPIOController() {
    // GPIO setup
    wiringPiSetupPhys();    
}

GPIOController::~GPIOController() {

}

GPIOController& GPIOController::getInstance() {
    static GPIOController instance;     // Instantiated on first use.
    return instance;                    // Guaranteed to be destroyed.
}

void GPIOController::setMode(int pin, int mode) {
    pinMode(pin, mode);
}

void GPIOController::setPullUpDn(int pin, int pud) {
    pullUpDnControl(pin, pud);
}

void GPIOController::write(int pin, int value) {
    digitalWrite(pin, value);
}

uint8_t GPIOController::read(int pin) {
    return digitalRead(pin);
}