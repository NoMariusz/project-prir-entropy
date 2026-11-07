CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I../common

entropy_seq: main.cpp ../common/entropy_common.hpp
	$(CXX) $(CXXFLAGS) -o $@ main.cpp

clean:
	rm -f entropy_seq

.PHONY: clean
