#include "pa1616.hpp"

ofstream errorLog("errors.log");

int main(int argc, char **argv)
{
	cerr.rdbuf(errorLog.rdbuf());
	
	int32_t fd;
	char *buffer = (char *)calloc(GPS_MSG_SIZE, sizeof(char));
	uint8_t nbytes;
	char *field[GPS_PARSED_MSG_NUM_FIELDS];
	uint8_t GPS_Valid = 1;
	GPSPkg gps;

// ------BEGIN EARLY DEBUG PHASE WITH GIVEN BUFFER
	sprintf(buffer, "$GNGGA,165006.000,2241.9107,N,12017.2383,E,1,14,0.79,22.6,M,18.5,M,,*42");
	
	if (strncmp(&buffer[3], "GGA", 3) == 0) {
		packageGPSData(buffer, field, &gps);

		if (DEBUG) {
			cout << "UTC Time  :" << field[1] << endl;
			cout << "Latitude  :" << gps.latitude << endl;
			cout << "Longitude :" << gps.longitude << endl;
			cout << "Altitude  :" << field[9] << endl;
			cout << "Satellites:" << field[7] << endl;
		}
	}
	if (strncmp(&buffer[3], "RMC", 3) == 0) {
			packageGPSData(buffer, field, &gps);

			if (DEBUG) {
				cout << "Speed     :" << field[7] << endl;
				cout << "UTC Time  :" << field[1] << endl;
				cout << "Date      :" << field[9] << endl;
			}

			if (SetTime(field[9],field[1]) < 0) {
				GPS_Valid = 0;
			}
	}
// ------END EARLY DEBUG PHASE WITH GIVEN BUFFER

	if ((fd = OpenGPSPort(UART_DEV)) < 0)
	{
			GPS_Valid = 0;
			return -1;
	}

	if (obtainFix(fd, &buffer) < 0) {
			GPS_Valid = 0;
			return -1;
	}

	do {
			if (nbytes == 0) {
				cerr << "PA1616: No communication from GPS module." << endl;
				GPS_Valid = 0;
			} else {
				buffer[nbytes - 1] = '\0';
				if (DEBUG)
						cout << "[" << buffer << "]" << endl;
				if (!checksum_valid(buffer)) {
					if ((strncmp(buffer, "$GP", 3) == 0) |
						(strncmp(buffer, "$GN", 3) == 0)) {

						if (strncmp(&buffer[3], "GGA", 3) == 0) {
							packageGPSData(buffer, field, &gps);

							if (DEBUG) {
								cout << "UTC Time  :" << field[1] << endl;
								cout << "Latitude  :" << gps.latitude << endl;
								cout << "Longitude :" << gps.longitude << endl;
								cout << "Altitude  :" << field[9] << endl;
								cout << "Satellites:" << field[7] << endl;
							}
						}
						
						if (strncmp(&buffer[3], "RMC", 3) == 0) {
								packageGPSData(buffer, field, &gps);
			
								if (DEBUG) {
									cout << "Speed     :" << field[7] << endl;
									cout << "UTC Time  :" << field[1] << endl;
									cout << "Date      :" << field[9] << endl;
								}

								if (SetTime(field[9],field[1]) < 0) {
									GPS_Valid = 0;
								}
						}
					}
				}
			}
			if ((nbytes = read(fd, &buffer, sizeof(buffer))) < 0) {
					cerr << "PA1616: Could not read from GPS." << endl;
					GPS_Valid = 0;
					break;
			}
	} while(1);

	if (close(fd) < 0) {
		cerr << "PA1616: Could not close device file." << endl;
		GPS_Valid = 0;
		return -1;
	}
	free(buffer);
	errorLog.close();
	return 0;
}