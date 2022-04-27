/**
 * @file SHT30.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief Implementation of SHT30.hpp module wrapper
 * @version 0.1
 * @date 2022-04-14
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "SHT30.hpp"

#include <stdexcept>
#include <unistd.h>
#include <wiringPiI2C.h>


SHT30::SHT30() {
    sht30_fd = wiringPiI2CSetup(SHT30_I2C_ADDRESS);
    
    // Test for connection
    if (sht30_fd == -1)
        throw std::runtime_error("Failed to communicate with SHT30 module!");
}

SHT30::~SHT30() {
    close(sht30_fd);
}

SHT30& SHT30::getInstance()  {
    static SHT30 instance;      // Instantiated on first use.
    return instance;            // Guaranteed to be destroyed.
} 

// TODO: Implementing CRC check
uint8_t SHT30::CRC() {
    uint8_t crc = 0;
    // const uint8_t POLYNOMIAL = 0x31;
    // uint8_t crc = 0xFF;
    // uint32_t i, j;

    // if (DEBUG) {
	// cout << "data[0]=" << hex << unsigned(data[0]) << endl;
    //     cout << "data[1]=" << unsigned(data[1]) << endl;
    //     cout << "data[2]=" << unsigned(data[2]) << endl;
    // }
   
    // for (i=0; i<len; ++i) 
    // {
    //     crc ^= *data++;
    //     if (DEBUG) {
    //         cout << "i=" << i << endl;
    //         cout << "crc=" << hex << unsigned(crc) << endl;
    //     }
 
    //     for (j=0; j<8; ++j) 
    //     {
    //         crc = ( crc & 0x80 )? (crc << 1) ^ POLYNOMIAL: (crc << 1);
    //     }
    // }
    // return crc;

    return crc;
}

SHT30::RawMeasurement SHT30::getRawMeasurement() {
    RawMeasurement rawMeasurement; 

    // Send single shot command
    char command[] = SHT30_SINGLE_SHOT_REP_HIGH;
    write(sht30_fd, command, SHT30_COMMAND_SIZE);

    // Wait for measurement
    sleep(1);

    // Read pair data
    ssize_t bytes_read = read(sht30_fd, &rawMeasurement, SHT30_DATA_SIZE);

    if (bytes_read != SHT30_DATA_SIZE) {
        throw std::length_error("Failed to reading temperature and humidity data from SHT30!");
    }

    // Perform CRC check. If fail, request new measurement

    return rawMeasurement;
}

SHT30::Measurement SHT30::getMeasurement() {
    RawMeasurement rm = getRawMeasurement();

    uint16_t rawTemperature = ((u_int16_t)rm.TemperatureH << 8) + (u_int16_t)rm.TemperatureL;
    uint16_t rawHumidity = (uint16_t)(rm.HumidityH << 8) + (uint16_t)rm.HumidityL;

    Measurement measurement;
    measurement.Temperature_F = -49.0f + (315.0f * rawTemperature) / 65535.0f;
    measurement.Temperature_C = -45.0f + (175.0f * rawTemperature) / 65535.0f;
    measurement.Humidity = (100.0f * rawHumidity) / 65535.0f;

    return measurement;
}
