/**
 * @file lora_send_test.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2022-04-27
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <sstream>
#include <iostream>
#include <unistd.h>

#include "../../src/GPIOController.hpp"

#define MUX_A_PIN 26
#define MUX_B_PIN 32

void setup() {
    
}

int main() {
    std::cout << " ---- LoRa Console -------" << std::endl;
    
    // ---- Setup --------------------------
    
    GPIOController& gpio = GPIOController::getInstance();

    // Setup UART mux and select GPS
    gpio.setMode(MUX_A_PIN, OUTPUT);
    gpio.setMode(MUX_B_PIN, OUTPUT);

    // LoRa E5 module setup
    gpio.setMode(36, OUTPUT);
    gpio.setMode(38, OUTPUT);

    gpio.write(36, HIGH);
    gpio.write(38, HIGH);
    
    sleep(1);
    gpio.write(38, LOW);
    sleep(1);
    gpio.write(38, HIGH);

    gpio.write(MUX_A_PIN, LOW);
    gpio.write(MUX_B_PIN, HIGH);

    // ---- Connection test ----------------

    gpio.serialSend("AT\n");
    usleep(20000);
    std::cout << gpio.serialReadLine().str() << std::endl;

    // ---- Continuous communication loop --

    std::string line;
    while (1) {
        std::getline(std::cin, line);

        if (line.empty())
            continue;

        gpio.serialSend(line);
        sleep(1);

        do {
            line = gpio.serialReadLine().str();

            std::cout << line << std::endl;
        } while (!line.empty());
    }
}