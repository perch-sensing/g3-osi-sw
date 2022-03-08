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
		m.def("packageGPSData", &packageGPSData, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Package necessary GPS data for transmission over LoRa");
		m.def("setTime", &setTime, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Parse GPS time");
		m.def("closeGPSPort", &closeGPSPort, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Close device file");
}

/* Open device file for GPS reading
 *
 * @param {const char *} devname - name of device file for GPS
 *
 * @return {int32_t} File descriptor, or -1 for open error
 */
int32_t openGPSPort(const char *devname)
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

	wiringPiSetup();
	pinMode(FIX_GPIO_PIN, INPUT);

	return fd;
}

string extractMsg(string buffString) {
	int16_t beginIndex = 0;
	if (DEBUG)
	    cout << "buffString size (extractMsg): " << buffString.size() << endl;
	while ((beginIndex = buffString.find_first_of('$', beginIndex)) >= 0) {
		if (DEBUG)
		    cout << "beginIndex: " << unsigned(beginIndex) << endl;
		if ((buffString.find("GN", beginIndex) == (beginIndex + 1)) ||
		    (buffString.find("GP", beginIndex) == (beginIndex + 1))) {
			if (DEBUG)
			    cout << "GN or GP detected." << endl;
			if ((buffString.find("GGA", beginIndex) == (beginIndex + 3)) ||
			    (buffString.find("RMC", beginIndex) == (beginIndex + 3))) {
				if (DEBUG)
				    cout << "GGA or RMC detected." << endl;
				int16_t endIndex;
				if ((endIndex = buffString.find_first_of(10, beginIndex)) > 0) {
					uint16_t msgLen = endIndex - beginIndex;
					return buffString.substr(beginIndex, msgLen);
				}
			}
		}
		beginIndex++;
	}
	return string("");
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
	if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
		if (write(fd, PMTK_CMD_COLD_START, strlen(PMTK_CMD_COLD_START)) < 0) {
				cerr << "PA1616: Could not write to GPS." << endl;
				if (close(fd) < 0) {
					cerr << "PA1616: Could not close device file." << endl;
				}
				return -1;
		}
		sleep(35);
		if ((nbytes = read(fd, buff, GPS_MSG_SIZE)) < 0) {
				for (uint8_t i = 0; i < 25; i += 5) {
						sleep(5);
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
	string buffString(buff);
	string extString = extractMsg(buffString);
	if (DEBUG)
		cout << "extString: " << extString << endl;
	if (extString.size() > 0) {
		if (PySequence_SetItem(bufferObj, 0, PyUnicode_FromString(extString.c_str())) < 0) {
			cerr << "PA1616: PyObject for obtained fix cannot be populated." << endl;
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
 *
 * @return {int8_t} 0 upon successful data packaging, -1 otherwise
 */
int8_t packageGPSData(char *buffer, py::object field, GPSPkg& data) {
	int8_t i = parse_comma_delimited_str(buffer, field, GPS_PARSED_MSG_NUM_FIELDS);

	if (i < 0) {
			cerr << "PA1616: Failed to package GPS data." << endl;
			return -1;
	}
        PyObject* fields = field.ptr();
	data.latitude = PyBytes_AsString(PySequence_GetItem(fields, 2));
	data.longitude = PyBytes_AsString(PySequence_GetItem(fields, 4));

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