/**
 * @file GPIOController.cpp
 * @author Diego Andrade (bets636@gmail.com)
 * @brief GPIOController interface implementation
 * @version 0.1
 * @date 2022-04-21
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "GPIOController.hpp"


GPIOController::GPIOController() {
    // GPIO setup with physical pin numbering mapping
    wiringPiSetupPhys(); 

    serial_fd   = 0;   
    serial_baud = 0;
}

GPIOController::~GPIOController() {
    // Closing resources
    serialClose(serial_fd);
    serial_fd   = 0;
    serial_baud = 0;
}

GPIOController& GPIOController::getInstance() {
    static GPIOController instance;     // Instantiated on first use.
    return instance;                    // Guaranteed to be destroyed.
}

// ---- Digital Functions ------------------

void GPIOController::setMode(int pin, int mode) {
    pinMode(pin, mode);
}

void GPIOController::setPullUpDn(int pin, int pud) {
    pullUpDnControl(pin, pud);
}


void GPIOController::write(int pin, int value) {
    digitalWrite(pin, value);
}

uint8_t GPIOController::read(int pin) {
    return digitalRead(pin);
}

// ---- Serial Functions -------------------

void GPIOController::configureSerial(uint16_t baudrate) {
    if (serial_fd == 0) {   
        serial_fd = serialOpen(SERIAL_DEVICE_PATH, baudrate);
        serial_baud = baudrate;
    } 
    
    if ((serial_fd != 0) && (serial_baud != baudrate)) {
        serialClose(serial_fd);

        serial_fd = serialOpen(SERIAL_DEVICE_PATH, baudrate);
        serial_baud = baudrate;
    }
}

int GPIOController::serialReadChar(uint16_t baudrate) {
    // Configure serial 
    configureSerial(baudrate);

    return serialGetchar(serial_fd);
}

std::stringstream GPIOController::serialReadLine(uint16_t baudrate) {
    // Line buffer
    std::stringstream ss;
    
    // Configure serial 
    configureSerial(baudrate);

    // Get line
    char data;
    while (serialDataAvail(serial_fd) > 0) {
        data = serialGetchar(serial_fd);

        // Chars to ignore
        if (data == '\r')
            continue;

        // Line read when char is return or newline
        if (data == '\n')
            break;

        // Build line
        ss << data;
    }

    return ss;
}

void GPIOController::serialSend(uint16_t baudrate, std::string message) {
    // Configure serial 
    configureSerial(baudrate);
    
    // Add terminator to message
    if (message.back() != '\n') {
        message.append("\n");
    }

    // Send command
    serialPuts(serial_fd, message.c_str());
}

// ---- UART Mux Functions -----------------
void GPIOController::selectUART(UART uart) {

    
}
