#include "sht30.hpp"

ofstream errorLog("errors.log");

int main() 
{
		//redirect cerr output to error log file
		cerr.rdbuf(errorLog.rdbuf());
	
    int32_t file = initialize();
    if (file >= 0) {
        uint16_t temp = 0;
        //
        float temp_hum_arr[TH_NUM_FIELDS] = {0.0};
        uint16_t count = 0;
        do {
	    if (processData(file, &temp, temp_hum_arr) >= 0) {
            // set data for LoRa transmission
						if (DEBUG) {
								cout << endl << "Temperature: " << temp << endl;
								cout << "Temperature (C): " << (double)temp_hum_arr[0] << endl;
								cout << "Temperature (F): " << (double)temp_hum_arr[1] << endl;
								cout << "Humidity: " << (double)temp_hum_arr[2] << endl;
						}
        }
	sleep(2);
	} while (++count < 10);
	close(file);
    }
    errorLog.close();
    return 0;
}