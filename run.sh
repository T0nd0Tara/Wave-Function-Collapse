#!/bin/bash

g++ -I ~/code-libs/olcPixelGameEngine -o main main.cpp -lX11 -lGL -lpthread -lpng -lstdc++fs -std=c++17 -g -Wall -Wextra -Wno-unknown-pragmas -Wno-unused-parameter
./main
