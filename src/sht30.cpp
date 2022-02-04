#include "sht30.hpp"

/* Initialize SHT30
 * 
 * @return {int32_t} File descriptor, or -1 for initialization error
 */
int32_t initialize() {
    int32_t file;

    // Check if file is open
    if (open_file() < 0) {
        return -1;
    }

    // Get I2C device
    ioctl(file, I2C_SLAVE, I2C_ADR);

    // Send measurement command (CLK_STRCH_EN)
    // High repeatability measurement (CLK_STRCH_EN_HIGH)
    char config[2] = {0};
    config[0] = CLK_STRCH_EN;
    config[1] = CLK_STRCH_EN_HIGH;

    if (write(file, config, 2) < 0) {
        if (DEBUG) {
            cout << "SHT30: Could not send command." << endl;
        }
        cerr << "SHT30: Could not send command." << endl;
        return -1;
    }

    sleep(1);

    return file;
}

/* Open file for preparation in reading SHT30 data
 * 
 * @return {int32_t} File descriptor, or -1 for error in opening file
 */
int32_t open_file() {
    int32_t file;

    if ((file = open(BUS, O_RDWR)) < 0) {
        if (DEBUG) {
            cout << "SHT30: Could not open device file." << endl;
        }
        cerr << "SHT30: Could not open device file." << endl;
        return -1;
    }

    return file;
}

/* Read SHT30 data
 *
 * @param {int32_t} file - File descriptor for i2c-1
 * @param {uint8_t**}  data - buffer for 6-byte data
 * 		Ordering is Temp msb, Temp lsb, Temp CRC, Humididty msb, Humidity lsb, Humidity CRC
 * @return {int8_t} 0 upon successful reading, -1 otherwise
 */
int8_t readData(int32_t file, uint8_t** data) {
	  if(read(file, *data, TH_DATA_SIZE) != TH_DATA_SIZE)
	  {
        if (DEBUG) {
            cout << "SHT30: Data could not be read." << endl;
        }
		    cerr << "SHT30: Data could not be read." << endl;
				return -1;
	  }
		return 0;
}

/* Check if data is corrupted using CRC8 algorithm
 *
 * @param {const uint8_t*} data - pointer to data buffer
 * @param {uint32_t} data - size of buffer 
 *
 * @return {uint8_t} CRC remainder
 */
uint8_t CRC8(const uint8_t *data, uint32_t len)
{
    const uint8_t POLYNOMIAL = 0x31;
    uint8_t crc = 0xFF;
    int i, j;
    cout << "data[0]=" << hex << unsigned(data[0]) << endl;
    cout << "data[1]=" << unsigned(data[1]) << endl;
    cout << "data[2]=" << unsigned(data[2]) << endl;
   
    for (i=0; i<len; ++i) 
    {
        crc ^= *data++;
        if (DEBUG) {
            cout << "i=" << i << endl;
            cout << "crc=" << hex << unsigned(crc) << endl;
        }
 
        for (j=0; j<8; ++j) 
        {
            crc = ( crc & 0x80 )? (crc << 1) ^ POLYNOMIAL: (crc << 1);
        }
    }
    return crc;
}

/* Read and convert data from SHT30
 *
 * @param {int32_t} file - file descriptor
 * @param {uint16_t*} temp - pointer to temperature variable
 * @param {float**} temp_hum_arr - pointer to array of Celsius temperature, 
 *                                 Fahrenheit temperature, and humidity variable
 * 
 * @return {int8_t} 0 upon successful data collection, otherwise -1
 */
int8_t processData(int32_t file, uint16_t* temp, float** temp_hum_arr) {
    uint8_t *data = new uint8_t[6];

    if(readData(file, &data) < 0) {
        if (DEBUG) {
            cout << "SHT30: Data could not be read." << endl;
        }
        cerr << "SHT30: Data could not be read." << endl;
        return -1;
    }

    if (CRC8(data, 3) != 0) {
        if (DEBUG) {
            cout << "SHT30: Temperature data is corrupted." << endl;
        }
        cerr << "SHT30: Temperature data is corrupted." << endl;
        return -1;
    }

    if (CRC8(data + 3, 3) != 0) {
        if (DEBUG) {
            cout << "SHT30: Humidity data is corrupted." << endl;
        }
        cerr << "SHT30: Humidity data is corrupted." << endl;
        return -1;
    }

    // Convert the data
    *temp = (data[0] * 256 + data[1]);
    (*temp_hum_arr)[0] = -45 + (175 * (*temp) / 65535.0);
    (*temp_hum_arr)[1] = -49 + (315 * (*temp) / 65535.0);
    (*temp_hum_arr)[2] = 100 * (data[3] * 256 + data[4]) / 65535.0;

    delete data;
    return 0;
}