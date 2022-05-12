#include "sht30.hpp"


PYBIND11_MODULE(sht30, m) {
    m.doc() = "pybind11 sht30 plugin";

    m.def("initialize", &initialize, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Initialize SHT30");
    m.def("readData", &readData, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Read SHT30 data");
    m.def("processData", &processData, py::call_guard<py::scoped_ostream_redirect, py::scoped_estream_redirect>(), "Read and convert data from SHT30");

}

/* Initialize SHT30
 * 
 * @param {const char *} bus - device file name
 * 
 * @return {int32_t} File descriptor, or -1 for initialization error
 */
int32_t initialize(const char *bus) {
    int32_t file;

    // Check if file is open
    if ((file = open_file(bus)) < 0) {
        return -1;
    }

    // Get I2C device
    ioctl(file, I2C_SLAVE, I2C_ADR);

    // Measurments per second configuration (CLK_MPS)
    // High repeatability configuration (CLK_REPEAT_HIGH)
    char config[2] = {0};
    config[0] = CLK_MPS;
    config[1] = CLK_REPEAT_HIGH;

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
 * @param {const char *} bus - device file name
 * 
 * @return {int32_t} File descriptor, or -1 for error in opening file
 */
int32_t open_file(const char *bus) {
    int32_t file;

    if ((file = open(bus, O_RDWR)) < 0) {
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
 * @param {py::object}  data - buffer for 6-byte data
 * 		Ordering is Temp msb, Temp lsb, Temp CRC, Humididty msb, Humidity lsb, Humidity CRC
 * @return {int8_t} 0 upon successful reading, -1 otherwise
 */
int8_t readData(int32_t file, py::object data) {
    uint8_t buff[TH_DATA_SIZE];
    PyObject* bufferObj = data.ptr();
    if(read(file, buff, TH_DATA_SIZE) != TH_DATA_SIZE) {
        return -1;
    }
    for (uint8_t i = 0; i < TH_DATA_SIZE; i++) {
        if (PySequence_SetItem(bufferObj, i, PyLong_FromUnsignedLong(buff[i])) < 0) {
            cerr << "SHT30: PyObject for raw data cannot be populated: at index "
                 << unsigned(i) << " - " << unsigned(buff[i]) << endl;
            return -1;
        }
    }
    return 0;
}

/* Check if data is corrupted using CRC8 algorithm
 *
 * @param {const uint8_t*} data - pointer to data buffer
 * @param {uint32_t} len - size of buffer 
 *
 * @return {uint8_t} CRC remainder
 */
uint8_t CRC8(const uint8_t *data, uint32_t len) {
    const uint8_t POLYNOMIAL = 0x31;
    uint8_t crc = 0xFF;
    uint32_t i, j;

    if (DEBUG) {
	cout << "data[0]=" << hex << unsigned(data[0]) << endl;
        cout << "data[1]=" << unsigned(data[1]) << endl;
        cout << "data[2]=" << unsigned(data[2]) << endl;
    }
   
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
 * @param {py:object} raw_data - pointer to array for raw bytes
 * @param {py::object} temp - pointer to temperature variable
 * @param {py::object} temp_hum_arr - pointer to array of Celsius temperature, 
 *                                 Fahrenheit temperature, and humidity variable
 * 
 * @return {int8_t} 0 upon successful data collection, otherwise -1
 */
int8_t processData(int32_t file, py::object raw_data, py::object temp, py::object temp_hum_arr) {
    PyObject* tmp = temp.ptr();
    PyObject* th_arr = temp_hum_arr.ptr();
    uint16_t tempH = 0;
    double th_temp_arr[TH_NUM_FIELDS] = {0.0};
    uint8_t data[TH_DATA_SIZE];
    
    // Read data from device file
    if(readData(file, raw_data) < 0) {
        if (DEBUG) {
            cout << "SHT30: Data could not be read." << endl;
        }
        cerr << "SHT30: Data could not be read." << endl;
        return -1;
    }
    for (uint8_t i = 0; i < TH_DATA_SIZE; i++) {
        if ((data[i] = (uint8_t)(PyLong_AsUnsignedLong(PySequence_GetItem(raw_data.ptr(), i)))) == (unsigned long)-1) {
            cerr << "SHT30: PyObject conversion to unsigned long failed for raw data at index "
                 << unsigned(i) << "." << endl;
            return -1;
        }
    }

    // Check if temperature data is corrupted through checksum algorithm
    if (CRC8(data, 3) != 0) {
        if (DEBUG) {
            cout << "SHT30: Temperature data is corrupted." << endl;
        }
        cerr << "SHT30: Temperature data is corrupted." << endl;
        return -1;
    }

    // Check if humidity data is corrupted through checksum algorithm
    if (CRC8(data + 3, 3) != 0) {
        if (DEBUG) {
            cout << "SHT30: Humidity data is corrupted." << endl;
        }
        cerr << "SHT30: Humidity data is corrupted." << endl;
        return -1;
    }

    // Convert the data
    tempH = (data[0] * 256 + data[1]);
    th_temp_arr[0] = -45 + (175 * (tempH) / 65535.0);
    th_temp_arr[1] = -49 + (315 * (tempH) / 65535.0);
    th_temp_arr[2] = 100 * (data[3] * 256 + data[4]) / 65535.0;

    // Set raw temperature data into Python object
    if (PySequence_SetItem(tmp, 0, PyLong_FromLong((long)(tempH))) < 0) {
	cerr << "SHT30: PyObject for raw temperature data could not be populated." << endl;
	return -1;
    }

    // Set temperature and humidity data into Python object
    for (uint8_t i=0; i<TH_NUM_FIELDS; i++) {
        if (PySequence_SetItem(th_arr, i, PyFloat_FromDouble(th_temp_arr[i])) < 0) {
	    cerr << "SHT30: PyObject for data could not be populated."  << endl;
	    return -1;
	}
    }

    return 0;
}
