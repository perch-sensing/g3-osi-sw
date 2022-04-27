/**
 * @file GPIOController.hpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief GPIO interface for external modules to use
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdint>

class GPIOController {
    private:
        GPIOController();
        ~GPIOController();

    public:
        GPIOController(GPIOController const&)     = delete;
        void operator=(GPIOController const&)     = delete;
        
        /** @brief Get single instance of GPIO Controller
         * @return GPIOController& (GPIOController reference)  
         */
        static GPIOController& getInstance();

        // ---- GPIO wrapper ----------
        void setMode(int pin, int mode);
        void setPullUpDn(int pin, int pud);
        void write(int pin, int value);
        uint8_t read(int pin);
};
