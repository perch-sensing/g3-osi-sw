/**
 * @file SHT30.hpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief SHT30 module wrapper
 * @version 0.1
 * @date 2022-04-14
 * @note Built off of early work done by Jacqueline Nguyen (jacquelinen721@gmail.com) and 
 * Nikita Kilmov (nikklim16@gmail.com).
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <wiringPiI2C.h>

#define SHT30_I2C_ADDRESS 0x44

// ---------- Commands ----------
#define SHT30_COMMAND_SIZE 2
#define SHT30_DATA_SIZE 6   // Bytes
#define SHT30_SINGLE_SHOT {0x24, 0x00}

/**
 * @brief SHT30 temperature module 
 * @note Using Singleton pattern for implementation to since only one SHT30 module is connected
 */
class SHT30 {

    private:
        SHT30();
        ~SHT30();

        int sht30_fd;

        struct RawMeasurement {
            uint8_t TemperatureH;
            uint8_t TemperatureL;
            uint8_t CRC_Temp;
            uint8_t HumidityH;
            uint8_t HumidityL;
            uint8_t  CRC_Humidity;
        };

        RawMeasurement getRawMeasurement();
        uint8_t CRC();
        
    public:
        SHT30(SHT30 const&)             = delete;
        void operator=(SHT30 const&)    = delete;
        
        /** @brief Struct holding SHT30 data */
        struct Measurement {
            float Temperature_C;
            float Temperature_F;
            float Humidity;
        };
        
        /** @brief Get single instance of SHT30
         * @return SHT30& (SHT30 reference)  
         */
        static SHT30& getInstance();

        /** @brief Get a temperature and humidity sample from the SHT30
         * @return SHT30 measurement object, holding temperature and humidity data. 
         */
        Measurement getMeasurement();
};