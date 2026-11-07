CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -fopenmp -I../common

entropy_omp: main.cpp ../common/entropy_common.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

clean:
	rm -f entropy_omp

.PHONY: clean
