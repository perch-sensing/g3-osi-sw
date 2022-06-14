/**
 * @file perch_sense.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief Full run of G3 sensing
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "GPS/PA1616.hpp"
#include "LoRa/LoRa_E5.hpp"
#include "Temperature/SHT30.hpp"

#include <chrono>
#include <iostream>
#include <thread>
#include <unistd.h>

// NOTE: LoRa module does not like comma "," but dash "-" works!

int main() {
    // Setup
    LoRa_E5& lora = LoRa_E5::getInstance();
    // PA1616& gps = PA1616::getInstance();
    // SHT30& tempHum = SHT30::getInstance();

    // Start main logic
    // Is this hard coded loop or state machine

    // If GPS location hasn't been received for today (since pi power on for testing)
    // gps.powerOn();      // Turn on GPS and allow it time to get fix

    // SHT30::Measurement tempHum_reading = tempHum.getMeasurement();

    // Run image processing

    // Finally if GPS data not yet gotten
    // PA1616::RMC gps_data = gps.getLocation();

    // Send data

    // 1-1
    // 2-lat
    // 3-lon
    // 4-
    int sleep_time = 3075;

    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    lora.sendUnconfirmedMessage("1-1");             // Vision result
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

    lora.sendUnconfirmedMessage("2-3518.154");     // Lat
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));

    lora.sendUnconfirmedMessage("3-12039.400");    // Lon
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    
    lora.sendUnconfirmedMessage("4-25.3285");       // C
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    
    lora.sendUnconfirmedMessage("5-51.2459");       // RH
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    
    lora.sendUnconfirmedMessage("6-3.70");          // Battery
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    
    lora.sendUnconfirmedMessage("7-4.12");          // Solar
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_time));
    
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();


    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() << "[ns]" << std::endl;
    std::cout << std::endl;
    // Sleep
    
    for (int i = 0; i < 7; i++) {
        std::cout << lora.readLine().str() << std::endl;
        std::cout << lora.readLine().str() << std::endl;
        std::cout << lora.readLine().str() << std::endl;
    }

    return 0;
}