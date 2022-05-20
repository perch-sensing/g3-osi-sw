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

#define FLAG_PIN 23

int main(int argc, char **argv) {
    std::cout << " ---- LoRa send test ----" << std::endl;

    // Validate command line arguements
    if (argc > 3) {
        std::cout << "Incorrect number of arguments found: " << (argc - 1) << std::endl;
        std::cout << "Valid arguments number: 0 - 2" << std::endl;
        std::cout << "<data> <times>" << std::endl;
        return -1;
    }
    
    // Setup
    GPIOController& gpio = GPIOController::getInstance();
    gpio.setMode(FLAG_PIN, OUTPUT);
    gpio.write(FLAG_PIN, LOW);

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

    // Run
    std::string ret1, ret2, ret3;

    LoRa_E5& lora = LoRa_E5::getInstance();

    for ( ; repeat > 0; --repeat) {

        gpio.write(FLAG_PIN, HIGH);
        lora.sendUnconfirmedMessage(data);  
        ret1 = lora.readLine().str();       // Reading unconfirmed message response
        ret2 = lora.readLine().str();       // TODO: Check format or wait for done message
        ret3 = lora.readLine().str();
        gpio.write(FLAG_PIN, LOW);

        std::cout << ret1 << std::endl;
        std::cout << ret2 << std::endl;
        std::cout << ret3 << std::endl;
    }

}
