/**
 * @file GPIOController.hpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief GPIO interface for external modules to use, additionally controlling
 *        simple modules like the UART mux
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdint>
#include <sstream>
#include <wiringSerial.h>
#include <wiringPi.h>

// ---- Serial Configuration ---------------
#define SERIAL_DEVICE_PATH "/dev/serial0"
#define SERIAL_DEFAULT_BAUD 9600

// ---- Mux Configuration ------------------
#define SERIAL_MUX_A 26
#define SERIAL_MUX_B 32

class GPIOController {
    public:
        GPIOController(GPIOController const&)     = delete;
        void operator=(GPIOController const&)     = delete;
        
        /** @brief Get single instance of GPIO Controller
         * @return GPIOController& (GPIOController reference)  
         */
        static GPIOController& getInstance();

        // ---- Digital Interface ----------

        /**
         * @brief Set the pinmode of GPIO pin
         * 
         * @param pin The physical pin number
         * @param mode Valid mode: INPUT, OUTPUT, PWM_OUTPUT (BCM_GPIO 18), GPIO_CLOCK (BCM_GPIO 4)
         */
        void setMode(int pin, int mode);

        /**
         * @brief Set internal pull up or pull down resistor
         * 
         * @param pin The physical pin number
         * @param pud Valid: PUD_OFF, PUD_DOWN, and PUD_UP
         */
        void setPullUpDn(int pin, int pud);

        /**
         * @brief Digital write of OUTPUT pin
         * 
         * @param pin The physical pin number
         * @param value Valid values: HIGH or LOW
         */
        void write(int pin, int value);

        /**
         * @brief Digital read of pin
         * 
         * @param pin The physical pin number
         * @return uint8_t HIGH or LOW
         */
        uint8_t read(int pin);

        // ---- Serial Interface -----------

        enum UART { LOAD, GPS, LORA };

        void selectUART(UART uart);

        int serialReadChar(UART uart) { return serialReadChar(SERIAL_DEFAULT_BAUD, uart); }
        int serialReadChar(uint16_t baudrate, UART uart);

        std::stringstream serialReadLine(UART uart) { return serialReadLine(SERIAL_DEFAULT_BAUD, uart); }
        std::stringstream serialReadLine(uint16_t baudrate, UART uart);
       
        void serialSend(std::string message, UART uart) { serialSend(SERIAL_DEFAULT_BAUD, message, uart); }
        void serialSend(uint16_t baudrate, std::string message, UART uart);
    private:
        GPIOController();
        ~GPIOController();

        void configureSerial(uint16_t baudrate);

        // Serial info 
        int serial_fd;
        uint16_t serial_baud;

        UART serial_selected;
};
