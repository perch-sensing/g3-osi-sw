#include "pa1616.hpp"

ofstream errorLog("errors.log");

int main(int argc, char **argv)
{
	cerr.rdbuf(errorLog.rdbuf());
	
	int32_t fd;
	char buffer[GPS_MSG_SIZE];
	string field[GPS_PARSED_MSG_NUM_FIELDS];
	uint8_t GPS_Valid = 1;
	GPSPkg gps;

// ------BEGIN EARLY DEBUG PHASE WITH GIVEN BUFFER
	sprintf(buffer, "$GNGGA,165006.000,2241.9107,N,12017.2383,E,1,14,0.79,22.6,M,18.5,M,,*42");
	
	if (strncmp(&buffer[3], "GGA", 3) == 0) {
		packageGPSData(buffer, field, gps);

		if (DEBUG) {
			cout << "UTC Time  :" << field[1] << endl;
			cout << "Latitude  :" << gps.latitude << endl;
			cout << "Longitude :" << gps.longitude << endl;
			cout << "Altitude  :" << field[9] << endl;
			cout << "Satellites:" << field[7] << endl;
		}
	}
	if (strncmp(&buffer[3], "RMC", 3) == 0) {
			packageGPSData(buffer, field, gps);

			if (DEBUG) {
				cout << "Speed     :" << field[7] << endl;
				cout << "UTC Time  :" << field[1] << endl;
				cout << "Date      :" << field[9] << endl;
			}

			/* if (setTime(field[9],field[1]) < 0) {
				GPS_Valid = 0;
			}*/
	}
// ------END EARLY DEBUG PHASE WITH GIVEN BUFFER

	if (((fd = openGPSPort(UART_DEV)) < 0) || (obtainFix(fd, buffer) < 0))
	{
			GPS_Valid = 0;
			return -1;
	}

	int counter = 0;

	do {
			if (!strlen(buffer)) {
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
							packageGPSData(buffer, field, gps);

							if (DEBUG) {
								cout << "UTC Time  :" << field[1] << endl;
								cout << "Latitude  :" << gps.latitude << endl;
								cout << "Longitude :" << gps.longitude << endl;
								cout << "Altitude  :" << field[9] << endl;
								cout << "Satellites:" << field[7] << endl;
							}
						}
						
						if (strncmp(&buffer[3], "RMC", 3) == 0) {
								packageGPSData(buffer, field, gps);
			
								if (DEBUG) {
									cout << "Speed     :" << field[7] << endl;
									cout << "UTC Time  :" << field[1] << endl;
									cout << "Date      :" << field[9] << endl;
								}

								/*
								if (setTime(field[9],field[1]) < 0) {
									GPS_Valid = 0;
								}*/
						}
					}
				}
			}
			if (obtainFix(fd, buffer) < 0) {
				GPS_Valid = 0;
				break;
			}
			counter += 1;
	} while(counter < 10);

	if (closeGPSPort(fd) < 0) {
		GPS_Valid = 0;
		return -1;
	}

	errorLog.close();
	return 0;
}