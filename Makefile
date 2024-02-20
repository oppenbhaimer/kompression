CC=g++
CFLAGS=-std=c++17 -Wall

all: src/compress.cpp src/crc.cpp src/crc.hpp
	$(CC) $(CFLAGS) src/compress.cpp src/crc.cpp -o bin/compress
