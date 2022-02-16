#include "pa1616.hpp"

/* Open device file for GPS reading
 *
 * @param {const char *} devname - name of device file for GPS
 *
 * @return {int32_t} File descriptor, or -1 for open error
 */
int32_t OpenGPSPort(const char *devname)
{
	uint32_t fd;
	struct termios options;

	if ((fd = open(devname, O_RDWR | O_NOCTTY | O_NDELAY)) < 0) {
		cerr << "PA1616: Could not open device file." << endl;
		return -1;
	}

	// Set to blocking
	fcntl(fd, F_SETFL, 0);

	// Get port attributes
	tcgetattr(fd, &options);

	// Set input and output baud rates
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);

	// Set input modes
  options.c_iflag &= ~(IXON | IXOFF | IXANY);
	options.c_iflag |= ICRNL;
  options.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR);

	// Set 8 bits, no parity, 1 stop bit
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8;
  options.c_cflag &= ~CRTSCTS;
  options.c_cflag |= CREAD | CLOCAL;

	options.c_lflag &= ~(ECHO|ECHOE|ECHONL);
	options.c_lflag &= ~ICANON;
  options.c_lflag &= ~ISIG;
 
  options.c_oflag &= ~OPOST;
  options.c_oflag &= ~ONLCR;

	// Set port attributes
	tcsetattr(fd, TCSAFLUSH, &options);

	return fd;
}

/* Obtain fix from GPS
 *
 * @param {int32_t} fd - file descriptor of device file
 * @param {char**} buffer - GPS data container
 *
 * @return {int8_t} 0 upon successful data collection, -1 otherwise
 */
int8_t obtainFix(int32_t fd, char** buffer) {
	uint8_t nbytes;
	if ((nbytes = read(fd, &buffer, sizeof(buffer))) < 0) {
		if (write(fd, PMTK_CMD_COLD_START, strlen(PMTK_CMD_COLD_START)) < 0) {
				cerr << "PA1616: Could not write to GPS." << endl;
				if (close(fd) < 0) {
					cerr << "PA1616: Could not close device file." << endl;
				}
				return -1;
		}
		sleep(35);
		if ((nbytes = read(fd, &buffer, sizeof(buffer))) < 0) {
				for (uint8_t i = 0; i < 25; i += 5) {
						sleep(5);
						if ((nbytes = read(fd, &buffer, sizeof(buffer))) >= 0)
								break;
				}
		}
		if (nbytes < 0) {
				cerr << "PA1616: Could not read from GPS." << endl;
				if (close(fd) < 0) {
					cerr << "PA1616: Could not close device file." << endl;
				}
				return -1;
		}
	}
	return 0;
}

/* Convert hex character into an integer
 *
 * @param {char} c - hex character
 *
 * @return {int8_t} value of hex character, -1 otherwise
 */
int8_t hexchar2int(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return -1;
}

/* Convert hex into an integer
 *
 * @param {char*} c - character of hex
 *
 * @return {int16_t} value of hex, -1 otherwise
 */
int16_t hex2int(char *c)
{
	uint8_t value;
	int8_t retVal;
	retVal = hexchar2int(c[0]);
	if (retVal < 0)
		return retVal;
	value = retVal;
	value = value << 4;
	retVal = hexchar2int(c[1]);
	if (retVal < 0)
		return retVal;
	value += retVal;

	return int16_t(value);
}

/* Validate checksum from GPS data
 *
 * @param {char*} string - GPS data
 *
 * @return {int8_t} 0 upon matching checksum, -1 otherwise
 */
int8_t checksum_valid(char *string)
{
	char *checksum_str;
	uint16_t checksum;
	uint16_t calculated_checksum = 0;
	int16_t validCSRet;

	// Checksum is postcede by *
	checksum_str = strchr(string, '*');
	if (checksum_str != NULL){
		// Remove checksum from string
		*checksum_str = '\0';

		// Calculate checksum, starting after $ (i = 1)
		for (uint8_t i = 1; i < strlen(string); i++) {
			calculated_checksum = calculated_checksum ^ string[i];
		}
		validCSRet = hex2int((char *)checksum_str+1);
		if (validCSRet < 0) {
				cerr << "PA1616: Checksum could not be converted to an integer." << endl;
				return -1;		
		}
		checksum = uint16_t(validCSRet);
		if (DEBUG) {
				cout << "Checksum Str " << (char *)checksum_str+1 << ", Checksum " << checksum << ", " << "Calculated Checksum " << calculated_checksum << endl;
		}
		if (checksum == calculated_checksum) {
			return 0;
		}
	} else {
		cerr << "PA1616: Checksum missing or NULL NMEA message" << endl;
		return -1;
	}
	return -1;
}

/* Parse GPS data with comma delimiter
 *
 * @param {char*} string - GPS data
 * @param {char**} fields - data container that holds separate fields of GPS data
 * @param {uint8_t} max_fields - number of maximum fields within GPS data
 *
 * @return {int8_t} number of fields within GPS data
 */
uint8_t parse_comma_delimited_str(char *string, char **fields, uint8_t max_fields)
{
	uint8_t i = 0;
	fields[i++] = string;
	
	while ((i < max_fields) && NULL != (string = strchr(string, ','))) {
		*string = '\0';
		fields[i++] = ++string;
	}

	return --i;
}

/* Parse GPS time
 *
 * @param {char*} date - GPS date
 * @param {char*} time - GPS time
 *
 * @return {int8_t} 0 upon successful time set, -1 otherwise
 */
int8_t SetTime(char *date, char *time)
{
	struct timespec ts;
	struct tm * gpstime;
	time_t secs;
	char tempbuf[3];
	int8_t ret;

	gpstime = new tm();

	if (DEBUG)
 		cout << "GPS    UTC_Date " << date << ", UTC_Time " << time << endl;
	// GPS date has format of ddmmyy
	// GPS time has format of hhmmss.ss

	if ((strlen(date) != 6) | (strlen(time) != 9)) {
		cerr << "PA1616: No date or time fix." << endl;
		return -1;
	}

	// Parse day:
	strncpy(tempbuf, (char *)date, 2);
	tempbuf[2] = '\0';
	gpstime->tm_mday = atoi(tempbuf);

	// Parse month:
	strncpy(tempbuf, (char *)date+2, 2);
	tempbuf[2] = '\0';
	gpstime->tm_mon = atoi(tempbuf) - 1;

	// Parse year:
	strncpy(tempbuf, (char *)date+4, 2);
	tempbuf[2] = '\0';
	gpstime->tm_year = atoi(tempbuf) + 100;

	// Parse hour:
	strncpy(tempbuf, (char *)time, 2);
	tempbuf[2] = '\0';
	gpstime->tm_hour = atoi(tempbuf);

	// Parse minutes:
	strncpy(tempbuf, (char *)time+2, 2);
	tempbuf[2] = '\0';
	gpstime->tm_min = atoi(tempbuf);

	// Parse seconds:
	strncpy(tempbuf, (char *)time+4, 2);
	tempbuf[2] = '\0';
	gpstime->tm_sec = atoi(tempbuf);

	if (DEBUG)
		cout << "Converted UTC_Date " << gpstime->tm_mday << (gpstime->tm_mon)+1 << (gpstime->tm_year)%100 << ", UTC_Time " << gpstime->tm_hour << gpstime->tm_min << gpstime->tm_sec << endl;

	ts.tv_sec = mktime(gpstime);
	// Apply GMT offset to correct for timezone
	ts.tv_sec += gpstime->tm_gmtoff;

	if (DEBUG)
		cout << "Number of seconds since Epoch: " << ts.tv_sec << endl;

	ts.tv_nsec = 0;
	ret = clock_settime(CLOCK_REALTIME, &ts);
	if (ret)
		cerr << "PA1616: System clock could not be set." << endl;

	if (DEBUG) {
			clock_gettime(CLOCK_REALTIME, &ts);
			cout << "Number of seconds since Epoch: " << ts.tv_sec << endl;
			delete gpstime;
			gpstime = gmtime(&ts.tv_sec);
			cout << "System UTC_Date: " << gpstime->tm_mday << (gpstime->tm_mon)+1 << (gpstime->tm_year)%100 << ", ";
			cout << "UTC_Time " << gpstime->tm_hour << gpstime->tm_min << gpstime->tm_sec << endl << endl;
	}
	return ret;
}

/* Print fields of GPS data for debugging purposes
 *
 * @param {uint8_t} numfields - number of fields within GPS data 
 * @param {char**} fields - data container that holds separate fields of GPS data
 *
 * @return {void}
 */
void debug_print_fields(uint8_t numfields, char **fields)
{
	cout << "Parsed " << unsigned(numfields) << " fields" << endl;

	for (uint8_t i = 0; i <= numfields; i++) {
		cout << "Field " << unsigned(i)  << ": " << fields[i] << endl;
	}
}

/* Package necessary GPS data for transmission over LoRa
 *
 * @param {char*} buffer - GPS data
 * @param {char**} field - data container that holds separate fields of GPS data
 * @param {GPSPkg*} data - GPSPkg struct to hold necessary GPS data
 *
 * @return {void}
 */
void packageGPSData(char *buffer, char **field, GPSPkg* data) {
	uint8_t i = parse_comma_delimited_str(buffer, field, GPS_PARSED_MSG_NUM_FIELDS);

	sprintf(data->latitude, "%s", field[2]);
	sprintf(data->longitude, "%s", field[4]);

	debug_print_fields(i,field);
}