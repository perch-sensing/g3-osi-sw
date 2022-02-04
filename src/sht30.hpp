#ifndef SHT30_HPP_
#define SHT30_HPP_
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdint>
#include <iostream>
#include <fstream>

#define BUS "/dev/i2c-1"
#define I2C_ADR 0x44
#define CLK_STRCH_EN 0x2C
#define CLK_STRCH_EN_HIGH 0x06
#define TH_DATA_SIZE 6
#define TH_NUM_FIELDS 3
#define DEBUG 1

using namespace std;

int32_t initialize();
int32_t open_file();
int8_t readData(int32_t file, uint8_t** data);
uint8_t CRC8(const uint8_t* data, uint32_t len);
int8_t processData(int32_t file, uint16_t* temp, float** temp_hum_arr);

#endif