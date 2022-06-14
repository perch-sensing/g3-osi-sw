/**
 * @file LoRa_E5.hpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief LoRa E5 module wrapper
 * @version 0.1
 * @date 2022-04-26
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <string>
#include <sstream>

// ---- Hardware Configuration -------------
#define LORA_EN_PIN 36
#define LORA_RESET_PIN 38

class LoRa_E5 {
    private:
        LoRa_E5();
        ~LoRa_E5();
        
    public:
        static LoRa_E5& getInstance();

        LoRa_E5(LoRa_E5 const&)         = delete;
        void operator=(LoRa_E5 const&)  = delete;

        void configureDeviceEUI(std::string EUI);
        void configureAppEUI(std::string EUI);
        void configureAppKey(std::string KEY);

        void joinNetwork();

        void sendCommand(std::string command);

        void sendUnconfirmedMessage(std::string message);

        std::stringstream readLine();
};



