/* TODO:
  Implement GPS struct
*/

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
#include <cstdint>

#define DEBUG 1
#define UART_DEV "/dev/ttyAMA0"
#define PMTK_CMD_COLD_START "$PMTK103*30<CR><LF>"
#define GPS_MSG_SIZE 255
#define GPS_PARSED_MSG_NUM_FIELDS 20
#define GPS_LAT_SIZE 10
#define GPS_LON_SIZE 11

using namespace std;

typedef struct GPSPkg {
    char latitude[GPS_LAT_SIZE];
    char longitude[GPS_LON_SIZE];
} GPSPkg;

int8_t hexchar2int(char c);
int16_t hex2int(char *c);
int8_t checksum_valid(char *string);
uint8_t parse_comma_delimited_str(char *string, char **fields, uint8_t max_fields);
int32_t OpenGPSPort(const char *devname);
int8_t obtainFix(int32_t fd, char** buffer);
void packageGPSData(char *buffer, char **field, GPSPkg* data);
int8_t SetTime(char *date, char *time);
void debug_print_fields(uint8_t numfields, char **fields);

#endif