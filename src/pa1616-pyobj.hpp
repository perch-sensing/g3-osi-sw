#ifndef PA1616_HPP_
#define PA1616_HPP_
#include <iostream>
#include <fstream>
#include <string.h>  // or <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <cstdint>
#include <string>
#include <pybind11/pybind11.h>
#include <pybind11/iostream.h>
#include <pybind11/stl.h>

#define DEBUG 1
#define UART_DEV "/dev/ttyAMA0"
#define PMTK_CMD_COLD_START "$PMTK103*30\r\n"
#define GPS_MSG_SIZE 350
#define GPS_PARSED_MSG_NUM_FIELDS 20
#define RMC_DATE_LEN 6
#define RMC_TIME_LEN 10
#define GGA_VALID_IDX 6
#define RMC_VALID_IDX 2
#define PMTK_SYS_MSG "$PMTK010,002*2D\r\n"
#define NMEA_HEADER "$G"
#define PMTK_EN_ANT "$CDCMD,33,1*7C\r\n"
#define PMTK_DIS_ANT "$CDCMD,33,0*7D\r\n"
#define ANTENNA_ACK_HEADER "$PCD,11,"
#define ANTENNA_ACK_IDX 8

using namespace std;
namespace py = pybind11;

typedef struct GPSPkg {
    string latitude;
    string longitude;
} GPSPkg;

int8_t hexchar2int(char c);
int16_t hex2int(char *c);
bool enableAntenna(int32_t fd);
bool disableAntenna(int32_t fd);
bool validCheck(char c, uint8_t fieldIdx);
string extractMsg(string buffString);
int8_t checksum_valid(char *s);
int8_t parse_comma_delimited_str(char *s, py::object field, uint8_t max_fields);
int32_t openGPSPort(const char *devname);
int8_t obtainFix(int32_t fd, py::object buffer);
int8_t packageGPSData(char *buffer, py::object fields, GPSPkg& data);
int8_t setTime(char* date, char* time);
void debug_print_fields(uint8_t numfields, py::object fields);
int8_t closeGPSPort(int32_t fd);

#endif
