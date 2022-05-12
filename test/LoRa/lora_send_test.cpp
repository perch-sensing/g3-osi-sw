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

#include <sstream>
#include <iostream>
#include <unistd.h>

#include "../../src/LoRa/LoRa_E5.hpp"
#include "../../src/GPIOController.hpp"

#define MUX_A_PIN 26
#define MUX_B_PIN 32

void setup() {
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
}

int main(int argc, char **argv) {
    std::cout << " ---- LoRa send test ----" << std::endl;

    // Validate command line arguements
    if (argc > 3) {
        std::cout << "Incorrect number of arguments found: " << (argc - 1) << std::endl;
        std::cout << "Valid arguments number: 0 - 2" << std::endl;
        std::cout << "<data> <times>" << std::endl;
        return -1;
    }
    
    std::string data = "T";

    // Update data if passed in
    if (argc > 1) {
        data = argv[1];
    }

    int repeat = 1;

    // Update number of times to send if passed in 
    if (argc > 2) {
        repeat = std::stoi(argv[2]);
    }

    std::cout << "Sending: " << data << std::endl;
    std::cout << "Repeat:  " << repeat << std::endl;
    std::cout << std::endl;

    // Setup LoRa module
    setup();

    // Run
    std::string ret;

    GPIOController& gpio = GPIOController::getInstance();
    LoRa_E5& lora = LoRa_E5::getInstance();

    for ( ; repeat > 0; --repeat) {
        lora.sendUnconfirmedMessage(data);
        sleep(5);

        do {
            ret = gpio.serialReadLine().str();
            std::cout << ret << std::endl;
        } while (!ret.empty());
    }

}