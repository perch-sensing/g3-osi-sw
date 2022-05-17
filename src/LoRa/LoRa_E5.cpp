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

#include <unistd.h>

#include "../GPIOController.hpp"

LoRa_E5::LoRa_E5() {
    // Setup LoRa pins
    GPIOController& gpio = GPIOController::getInstance();
    gpio.setMode(LORA_EN_PIN, OUTPUT);
    gpio.setMode(LORA_RESET_PIN, OUTPUT);

    gpio.write(LORA_EN_PIN, HIGH);
    gpio.write(LORA_RESET_PIN, HIGH);
    
    sleep(1);
    gpio.write(LORA_RESET_PIN, LOW);
    sleep(1);
    gpio.write(LORA_RESET_PIN, HIGH);
}

LoRa_E5::~LoRa_E5() {
    
}

LoRa_E5& LoRa_E5::getInstance() {
    static LoRa_E5 instance;
    return instance;
}

void LoRa_E5::configureDeviceEUI(std::string EUI) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+ID=DevEui, " + EUI, GPIOController::LORA);
}

void LoRa_E5::configureAppEUI(std::string EUI) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+ID=AppEui, " + EUI, GPIOController::LORA);
}

void LoRa_E5::configureAppKey(std::string KEY) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+KEY=APPKEY, " + KEY, GPIOController::LORA);
}

void LoRa_E5::joinNetwork() {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+JOIN", GPIOController::LORA);
}

void LoRa_E5::sendCommand(std::string command) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend(command, GPIOController::LORA);
}


void LoRa_E5::sendUnconfirmedMessage(std::string message) {
    GPIOController& gpio = GPIOController::getInstance();
    gpio.serialSend("AT+MSG=" + message, GPIOController::LORA);
}

std::stringstream LoRa_E5::readLine() {
    GPIOController& gpio = GPIOController::getInstance();
    return gpio.serialReadLine(GPIOController::LORA);
}
