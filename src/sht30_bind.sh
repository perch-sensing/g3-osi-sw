g++ -O3 -Wall -shared -std=c++11 -fPIC $(python3 -m pybind11 --includes) sht30.cpp -o sht30$(python3-config --extension-suffix)