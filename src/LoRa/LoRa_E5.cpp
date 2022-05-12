/**
 * @file LoRa_E5.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief LoRa_E5 implementation
 * @version 0.1
 * @date 2022-04-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "LoRa_E5.hpp"

#include "../GPIOController.hpp"

LoRa_E5::LoRa_E5() {

}

LoRa_E5::~LoRa_E5() {
    
}

LoRa_E5& LoRa_E5::getInstance() {
    static LoRa_E5 instance;
    return instance;
}

void LoRa_E5::configureDeviceEUI(std::string EUI) {
    GPIOController& gpio = GPIOController::getInstance();

    gpio.serialSend("AT+ID=DevEui, " + EUI);
}

void LoRa_E5::configureAppEUI(std::string EUI) {
    GPIOController& gpio = GPIOController::getInstance();

    gpio.serialSend("AT+ID=AppEui, " + EUI);
}

void LoRa_E5::configureAppKey(std::string KEY) {
    GPIOController& gpio = GPIOController::getInstance();

    gpio.serialSend("AT+KEY=APPKEY, " + KEY);
}

void LoRa_E5::joinNetwork() {
    GPIOController& gpio = GPIOController::getInstance();

    gpio.serialSend("AT+JOIN");
}

void LoRa_E5::sendUnconfirmedMessage(std::string message) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+MSG=" + message);
}
