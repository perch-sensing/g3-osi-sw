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

#include "../../src/LoRa/LoRa_E5.hpp"

int main() {
    std::cout << " ---- LoRa Console -------" << std::endl;
    
    // ---- Setup --------------------------
    LoRa_E5& lora = LoRa_E5::getInstance();

    // ---- Connection test ----------------
    lora.sendCommand("AT\n");
    usleep(20000);
    std::cout << lora.readLine().str() << std::endl;

    // ---- Continuous communication loop --
    std::string line;
    while (1) {
        std::getline(std::cin, line);

        if (line.empty())
            continue;

        lora.sendCommand(line);
        sleep(1);

        do {
            line = lora.readLine().str();
            std::cout << line << std::endl;
        } while (!line.empty());
    }
}