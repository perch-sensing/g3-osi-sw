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
#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
#include <pybind11/stl.h>

#define BUS "/dev/i2c-1"
#define I2C_ADR 0x44
#define CLK_MPS 0x20
#define CLK_REPEAT_HIGH 0x32
#define TH_DATA_SIZE 6
#define TH_NUM_FIELDS 3
#define DEBUG 0

using namespace std;
namespace py = pybind11;
using namespace py;

int32_t initialize(const char *bus);
int32_t open_file(const char *bus);
int8_t readData(int32_t file, py::object data);
uint8_t CRC8(const uint8_t* data, uint32_t len);
int8_t processData(int32_t file, py::object raw_data, py::object temp, py::object temp_hum_arr);

#endif
