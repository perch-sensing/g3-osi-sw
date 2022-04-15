
BUILD_FLAGS := -lwiringPi -Wall -Wextra -Werror

All:
	echo "Building all"

SHT30.o:
	@mkdir -p build/Temperature
	g++ -c src/Temperature/SHT30.cpp -o build/Temperature/SHT30.o ${BUILD_FLAGS}

SHT30_Test: SHT30.o
	g++ -o build/Temperature/SH30_test test/Temperature/SHT30_test.cpp build/Temperature/SHT30.o ${BUILD_FLAGS}


clean:
	@rm -rf build