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
	g++ -c ./src/GPS/PA1616.cpp -o ./build/GPS/PA1616.o ${BUILD_FLAGS}

LoRa_E5.o: GPIOController.o ./src/LoRa/LoRa_E5.cpp
	@mkdir -p build/LoRa
	g++ -c ./src/LoRa/LoRa_E5.cpp -o ./build/LoRa/LoRa_E5.o ${BUILD_FLAGS}

Temperature_Test: SHT30.o
	@mkdir -p build/test
	g++ -o ./build/test/temperature_test ./test/Temperature/temperature_test.cpp ./build/Temperature/SHT30.o ${LIBS} ${BUILD_FLAGS}

GPS_Test: PA1616.o
	@mkdir -p build/test
	g++ -o ./build/test/location_test ./test/GPS/location_test.cpp ./build/GPS/PA1616.o ./build/GPIOController.o ${LIBS} ${BUILD_FLAGS}

LoRa_Test: LoRa_E5.o
	@mkdir -p build/test
	g++ -o ./build/test/lora_send_test ./test/LoRa/lora_send_test.cpp ./build/LoRa/LoRa_E5.o ./build/GPIOController.o ${LIBS} ${BUILD_FLAGS}
	g++ -o ./build/test/lora_console ./test/LoRa/lora_console.cpp ./build/LoRa/LoRa_E5.o ./build/GPIOController.o ${LIBS} ${BUILD_FLAGS}

clean:
	@rm -rf build