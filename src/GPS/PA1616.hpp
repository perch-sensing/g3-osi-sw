/**
 * @file PA1616.hpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief PA1616 GPS module wrapper
 * @version 0.1
 * @date 2022-04-14
 * @note Built off of early work done by Jacqueline Nguyen (jacquelinen721@gmail.com) and 
 * Nikita Kilmov (nikklim16@gmail.com).
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include <cstdint>
#include <string>

#define PA1616_BAUD_RATE 9600

// ---- Hardware Configuartion --------
#define PA1616_SW_N_EN_PIN 35
#define PA1616_NRESET_PIN 37
#define PA1616_FIX_PIN 40

/**
 * @brief PA1616 GPS module 
 * @note Using Singleton pattern for implementation since only one PA1616 module is connected
 */
class PA1616 {

    private:
        PA1616();
        ~PA1616();
        
        void powerOn();
        void powerOff();
    public:
        PA1616(PA1616 const&)             = delete;
        void operator=(PA1616 const&)     = delete;
        
        /** @brief Get single instance of PA1616 module
         * @return PA1616& (PA1616 reference)  
         */
        static PA1616& getInstance();

        typedef struct RMC {
            std::string UTC;
            std::string STATUS;
            std::string LAT;
            std::string N_S;
            std::string LON;
            std::string E_W;
        } RMC;

        RMC getLocation();
        RMC getLocation(uint8_t tries);
};