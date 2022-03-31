#include "pa1616.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#define PMTK_Q_EPO_INFO "$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n$PMTK607*33\r\n"

ofstream errorLog("errors.log");

int main() {
	//cerr.rdbuf(errorLog.rdbuf());
	
	int32_t fd;
	char buffer[GPS_MSG_SIZE+1] = "$CDCMD,33,1*7C\r\n";
	
	if ((fd = openGPSPort(UART_DEV)) < 0) {
	        cout << "Failed to open port." << endl;
		return -1;
	}
	cout << "fd: " << fd << endl;
	char buff[GPS_MSG_SIZE+1];
	
	if (1) {
	if (sendCommand(fd, buffer)) {
		if (!recvCommand(fd, buff))
			cout << "Failed to receive command." << endl;
	}
	else
		cout << "Failed to send	command." << endl;
	this_thread::sleep_for(chrono::milliseconds(500));
	}
	if (closeGPSPort(fd) < 0) {
		return -1;
	}
	errorLog.close();
	return 0;
}
