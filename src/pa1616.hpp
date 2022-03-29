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
#include <sys/ioctl.h>

#define DEBUG 1
#define UART_DEV "/dev/ttyAMA0"
#define PMTK_CMD_COLD_START "$PMTK103*30\r\n"
#define GPS_MSG_SIZE 350
#define GPS_PARSED_MSG_NUM_FIELDS 20
#define GPS_LAT_SIZE 10
#define GPS_LON_SIZE 11
#define FIX_COUNTER 3
#define FIX_TIMER_COUNTER 3

using namespace std;

typedef struct GPSPkg {
    char latitude[GPS_LAT_SIZE];
    char longitude[GPS_LON_SIZE];
} GPSPkg;

int8_t hexchar2int(char c);
int16_t hex2int(char *c);
int8_t checksum_valid(char *s);
uint8_t parse_comma_delimited_str(char *s, string fields[], uint8_t max_fields);
int32_t openGPSPort(const char *devname);
bool sendCommand(int32_t fd, char* st);
bool recvCommand(int32_t fd, char* st);
int8_t obtainFix(int32_t fd, char* buffer);
void packageGPSData(char *buffer, string fields[], GPSPkg& data);
int8_t setTime(char *date, char *time);
void debug_print_fields(uint8_t numfields, string fields[]);
int8_t closeGPSPort(int32_t fd);

#endif