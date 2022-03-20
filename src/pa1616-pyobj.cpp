#include "pa1616-pyobj.hpp"

PYBIND11_MODULE(pa1616_pyobj, m) {
    m.doc() = "pybind11 pa1616 plugin";

		py::class_<GPSPkg>(m,"GPSPkg")
				.def(py::init<>())
				.def_readwrite("latitude", &GPSPkg::latitude)
				.def_readwrite("longitude", &GPSPkg::longitude);
		m.def("openGPSPort", &openGPSPort, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Open device file for GPS reading"); 
		m.def("obtainFix", &obtainFix, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Obtain fix from GPS");
		m.def("checksum_valid", &checksum_valid, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Validate checksum from GPS data");
		m.def("parse_comma_delimited_str", &parse_comma_delimited_str, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Parse GPS data with comma delimiter");
		m.def("packageGPSData", &packageGPSData, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Package necessary GPS data for transmission over LoRa");
		m.def("setTime", &setTime, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Parse GPS time");
		m.def("closeGPSPort", &closeGPSPort, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Close device file");
		m.def("enableAntenna", &enableAntenna, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Send PMTK command to enable GPS external antenna");
		m.def("disableAntenna", &disableAntenna, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Send PMTK command to disable GPS external antenna");
}

/* Open device file for GPS reading
 *
 * @param {const char *} devname - name of device file for GPS
 *
 * @return {int32_t} File descriptor, or -1 for open error
 */
int32_t openGPSPort(const char *devname)
{
	int32_t fd;
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
	if (tcsetattr(fd, TCSAFLUSH, &options) != 0) {
		cerr << "PA1616: Failed to set port attributes." << endl;
		return -1;
	}

	return fd;
}

/* Wait for accumulation of full read buffer from serial port 
 *
 * @param {int32_t} fd - file descriptor
 *
 * @return {void}
 */
void GPSReadWait(int32_t fd) {
	int bytesAvail = 0;
	while (bytesAvail < GPS_MSG_SIZE) {
		ioctl(fd, FIONREAD, &bytesAvail);
	}
}

/* Send PMTK command to enable GPS external antenna
 *
 * @param {int32_t} fd - file descriptor
 *
 * @return {bool} true if antenna was successfully enabled, false otherwise
 */
bool enableAntenna(int32_t fd) {
	char buff[GPS_MSG_SIZE+1];
	int16_t nbytes;
	string buffString;
	int16_t beginIndex = 0;

	do {
		GPSReadWait(fd);
		if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
			cerr << "PA1616: Cannot read from GPS." << endl;
			return false;
		}
		if (DEBUG)
			cout << "nbytes (startup completion message): " << nbytes << endl << endl;
		else if (nbytes == 0) {
			cerr << "PA1616: Did not read anything from GPS." << endl;
			return false;
		}
		buff[nbytes] = '\0';
		buffString = buff;
		//if (DEBUG)
			//cout << "buffString: " << buffString << endl << endl;
	}
	while ((beginIndex = buffString.find(PMTK_SYS_MSG, beginIndex)) < 0 && (beginIndex = buffString.find(NMEA_HEADER, beginIndex+1)) < 0);

	if (DEBUG)
		cout << "beginIndex: " << beginIndex << endl << endl;
	
	if ((nbytes = write(fd, PMTK_EN_ANT, strlen(PMTK_EN_ANT))) < 0) {
		cerr << "PA1616: Cannot write to GPS." << endl;
		return false;
	}
	if (DEBUG)
		cout << "nbytes (sent enable antenna command): " << nbytes << endl << endl;
	
	GPSReadWait(fd);	
	if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
		cerr << "PA1616: Cannot read from GPS." << endl;
                return false;
        } 
        else if (nbytes == 0) {
                cerr << "PA1616: Did not read anything from GPS." << endl;
                return false;
        }
	if (DEBUG)
		cout << "nbytes (acknowledgement of received command): " << nbytes << endl << endl;
	
	beginIndex = 0;
	buff[nbytes] = '\0';
	buffString = buff;

	if ((beginIndex = buffString.find(ANTENNA_ACK_HEADER, beginIndex)) < 0) {
		cerr << "PA1616: Did not receive acknowledgement for command enabling the antenna." << endl;
		return false;
	}

	int16_t endIndex;
	
	if ((endIndex = buffString.find_first_of(10, beginIndex)) < 0) {
		cerr <<  "PA1616: Could not parse ANTENNA_ACK packet." << endl;
		return false;
	}
	uint16_t msglen = endIndex - beginIndex;
	string antenna_ack(buffString.substr(beginIndex, msglen));

	char antennaBuff[GPS_MSG_SIZE];
	strcpy(antennaBuff, antenna_ack.c_str());

	if (checksum_valid(antennaBuff) < 0) {
		cerr << "PA1616: Corrupted ANTENNA_ACK packet." << endl;
		return false;
	}

	if (DEBUG)
		cout << "antenna_ack: " << antenna_ack[ANTENNA_ACK_IDX] << endl << endl;
	
	if (antenna_ack[ANTENNA_ACK_IDX] == '1') {
		cerr << "PA1616: WARNING - Using internal antenna." << endl;
	}
	else if (antenna_ack[ANTENNA_ACK_IDX] == '3') {
                cerr << "PA1616: Active antenna shorted." << endl;
		return false;
        }
	
	return true;
}

/* Send PMTK command to disable GPS external antenna
 *
 * @param {int32_t} fd - file descriptor
 *
 * @return {bool} true if antenna was successfully disabled, false otherwise
 */
bool disableAntenna(int32_t fd) {
	char buff[GPS_MSG_SIZE+1] = PMTK_DIS_ANT;
        int16_t nbytes;

        if ((nbytes = write(fd, buff, strlen(buff))) < 0) {
                cerr << "PA1616: Cannot write to GPS." << endl;
                return false;
        }

	return true;
}

/* Obtain fix from GPS
 *
 * @param {int32_t} fd - file descriptor of device file
 * @param {char*} buffer - GPS data container
 *
 * @return {int8_t} 0 upon successful data collection, -1 otherwise
 */
int8_t obtainFix(int32_t fd, py::object buffer) {
	PyObject* bufferObj = buffer.ptr();
	char buff[GPS_MSG_SIZE+1];
        /*uint8_t counter = 0;
	while (counter < FIX_TIMER_COUNTER) {
		uint8_t sum = 0;

		for (uint8_t i=0; i < FIX_COUNTER; i++) {
			sum += digitalRead(FIX_GPIO_PIN);
			if (DEBUG)
			    cout << (int)(sum) << endl;
			this_thread::sleep_for(chrono::milliseconds(500));
		}

		if (!sum) {
			break;
		}

		counter++;
	}
	
	if (counter == FIX_TIMER_COUNTER) {
			cerr << "PA1616: Fix waiting timeout." << endl;
			return -1;
	}*/
	
	int16_t nbytes;
	GPSReadWait(fd);
	if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
		if (write(fd, PMTK_CMD_COLD_START, strlen(PMTK_CMD_COLD_START)) < 0) {
				cerr << "PA1616: Could not write to GPS." << endl;
				if (close(fd) < 0) {
					cerr << "PA1616: Could not close device file." << endl;
				}
				return -1;
		}
		sleep(35);
		GPSReadWait(fd);
		if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
			for (uint8_t i = 0; i < 25; i += 5) {
				sleep(5);
				GPSReadWait(fd);
				if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) >= 0)
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
        if (DEBUG)
		cout << "nbytes: " << unsigned(nbytes) << endl << endl;
	buff[nbytes] = '\0';
        if (DEBUG) {
		for (int i = 0; i < nbytes; i++) {
			if (buff[i] >= 0 && buff[i] < 128) {
				cout << buff[i];
			}
		}
		cout << endl;
	}

	if (nbytes > 0) {
		if (PySequence_SetItem(bufferObj, 0, PyUnicode_FromString(buff)) < 0) {
			cerr << "PA1616: PyObject for obtained fix cannot be populated." << endl;
			return -1;
		}
		return 0;
	}
	return -1;
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
 * @param {char*} s - GPS data
 *
 * @return {int8_t} 0 upon matching checksum, -1 otherwise
 */
int8_t checksum_valid(char *s)
{
	char *checksum_str;
	uint16_t checksum;
	uint16_t calculated_checksum = 0;
	int16_t validCSRet;

	// Checksum is postcede by *
	checksum_str = strchr(s, '*');
	if (checksum_str != NULL){
		// Remove checksum from string
		*checksum_str = '\0';

		// Calculate checksum, starting after $ (i = 1)
		for (uint8_t i = 1; i < strlen(s); i++) {
			calculated_checksum = calculated_checksum ^ s[i];
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
		cerr << "PA1616: Checksum missing or NULL NMEA message." << endl;
		return -1;
	}
	return -1;
}

/* Parse GPS data with comma delimiter
 *
 * @param {char*} s - GPS data
 * @param {py::object} field - PyObject representing list of strings that holds fields of GPS data
 * @param {uint8_t} max_fields - number of maximum fields within GPS data
 *
 * @return {int8_t} number of fields within GPS data upon success, -1 otherwise
 */
int8_t parse_comma_delimited_str(char *s, py::object field, uint8_t max_fields) {
	PyObject* fields = field.ptr();
	uint8_t i = 0;
	char* fields_cstr[max_fields];
	fields_cstr[i++] = s;
	
	while ((i < max_fields) && NULL != (s = strchr(s, ','))) {
		*s = '\0';
		fields_cstr[i++] = ++s;
	}
	for (uint8_t j=0; j < i; j++) {
			if (PySequence_SetItem(fields, j, PyBytes_FromString(fields_cstr[j])) < 0) {
					cerr << "PA1616: PyObject for parsed data cannot be populated: at index " 
					     << unsigned(j) << " - \'" << fields_cstr[j] << "\'" << endl;
					return -1;
			}
	}

	return signed(--i);
}

/* Parse GPS time
 *
 * @param {char*} date - GPS date
 * @param {char*} time - GPS time
 *
 * @return {int8_t} 0 upon successful time set, -1 otherwise
 */
int8_t setTime(char* date, char* time)
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
	// GPS time has format of hhmmss.sss

	if ((strlen(date) != RMC_DATE_LEN) | (strlen(time) != RMC_TIME_LEN)) {
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
 * @param {py::object} field - data container that holds separate fields of GPS data
 *
 * @return {void}
 */
void debug_print_fields(uint8_t numfields, py::object field)
{
	PyObject* fields = field.ptr();
	cout << "Parsed " << unsigned(numfields) << " fields" << endl;

	for (uint8_t i = 0; i <= numfields; i++) {
		cout << "Field " << unsigned(i)  << ": " << PyBytes_AsString(PySequence_GetItem(fields, i)) << endl;
	}
}

/* Package necessary GPS data for transmission over LoRa
 *
 * @param {char*} buffer - GPS data
 * @param {py::object} field - data container that holds separate fields of GPS data
 * @param {GPSPkg&} data - reference to GPSPkg struct to hold necessary GPS data
 * @param {int32_t} lat_idx - index of latitude field in "field"
 * @param {int32_t} lon_idx - index of longitude field in "field"
 *
 * @return {int8_t} 0 upon successful data packaging, -1 otherwise
 */
int8_t packageGPSData(char *buffer, py::object field, GPSPkg& data, int32_t lat_idx, int32_t lon_idx) {
	int8_t i = parse_comma_delimited_str(buffer, field, GPS_PARSED_MSG_NUM_FIELDS);

	if (i < 0) {
			cerr << "PA1616: Failed to package GPS data." << endl;
			return -1;
	}
        PyObject* fields = field.ptr();
	data.latitude = PyBytes_AsString(PySequence_GetItem(fields, lat_idx));
	data.longitude = PyBytes_AsString(PySequence_GetItem(fields, lon_idx));

	if (DEBUG)
		debug_print_fields(i,field);
	
	return 0;
}

/* Close device file
 *
 * @param {int32_t} fd - file descriptor for GPS device file
 *
 * @return {int8_t} 0 upon successful closing of device file, -1 for close error
 */
int8_t closeGPSPort(int32_t fd) {
	if (close(fd) < 0) {
		cerr << "PA1616: Could not close device file." << endl;
		return -1;
	}

	return 0;
}
