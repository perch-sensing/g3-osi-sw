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

    // Mux module setup
    pinMode(SERIAL_MUX_A, OUTPUT);
    pinMode(SERIAL_MUX_B, OUTPUT);

    selectUART(LOAD);

    serial_selected = LOAD;
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

int GPIOController::serialReadChar(uint16_t baudrate, UART uart) {
    // Configure serial 
    configureSerial(baudrate);
    selectUART(uart);

    return serialGetchar(serial_fd);
}

std::stringstream GPIOController::serialReadLine(uint16_t baudrate, UART uart) {
    // Line buffer
    std::stringstream ss;
    
    // Configure serial 
    configureSerial(baudrate);
    selectUART(uart);

    // Get line
    int data;

    do {
        data = serialGetchar(serial_fd);

        if (data != -1 && data != '\r' && data != '\n')     // Exclude chars
            ss << static_cast<char>(data);
    } while (data != -1 && data != '\n');

    return ss;
}

void GPIOController::serialSend(uint16_t baudrate, std::string message, UART uart) {
    // Configure serial 
    configureSerial(baudrate);
    selectUART(uart);
    
    // Add terminator to message
    if (message.back() != '\n') {
        message.append("\n");
    }

    // Send command
    serialPuts(serial_fd, message.c_str());
}

// ---- UART Mux Functions -----------------
void GPIOController::selectUART(UART uart) {

    // Do nothing if already selected
    if (uart == serial_selected)
        return;

    switch (uart) {
        case LOAD:
            digitalWrite(SERIAL_MUX_A, LOW);
            digitalWrite(SERIAL_MUX_B, LOW);
            break;

        case GPS:
            digitalWrite(SERIAL_MUX_A, HIGH);
            digitalWrite(SERIAL_MUX_B, LOW);
            break;

        case LORA:
            digitalWrite(SERIAL_MUX_A, LOW);
            digitalWrite(SERIAL_MUX_B, HIGH);
            break;


        default:
            digitalWrite(SERIAL_MUX_A, LOW);
            digitalWrite(SERIAL_MUX_B, LOW);
            break;
    }
    
    serial_selected = uart;
}
