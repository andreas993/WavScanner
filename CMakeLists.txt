cmake_minimum_required(VERSION 4.3)
project(WavScanner)

set(CMAKE_CXX_STANDARD 20)

add_executable(WavScanner main.cpp
        WavInfo.cpp
        WavInfo.h
        FileHandler.cpp
        FileHandler.h)
