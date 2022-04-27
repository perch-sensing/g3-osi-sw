LIBS        := -lwiringPi
BUILD_FLAGS := -Wall -Wextra -Werror

All: SHT30.o PA1616.o GPIOController.o
	@echo "\nAll modules built"

Test: Temp_Test GPS_Test
	@echo "\nAll test built"

GPIOController.o: ./src/GPIOController.cpp
	g++ -c ./src/GPIOController.cpp -o ./build/GPIOController.o ${LIBS} ${BUILD_FLAGS}

SHT30.o: ./src/Temperature/SHT30.cpp
	@mkdir -p build/Temperature
	g++ -c ./src/Temperature/SHT30.cpp -o ./build/Temperature/SHT30.o ${LIBS} ${BUILD_FLAGS}

PA1616.o: GPIOController.o ./src/GPS/PA1616.cpp
	@mkdir -p build/GPS
	g++ -c ./src/GPS/PA1616.cpp -o ./build/GPS/PA1616.o ${LIBS} ${BUILD_FLAGS}

Temperature_Test: SHT30.o
	@mkdir -p build/test
	g++ -o ./build/test/temperature_test ./test/Temperature/temperature_test.cpp ./build/Temperature/SHT30.o ${LIBS} ${BUILD_FLAGS}

GPS_Test: PA1616.o
	@mkdir -p build/test
	g++ -o ./build/test/location_test ./test/GPS/location_test.cpp ./build/GPS/PA1616.o ./build/GPIOController.o ${LIBS} ${BUILD_FLAGS}

clean:
	@rm -rf build