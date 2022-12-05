cmake_minimum_required(VERSION 3.20)
project(timestamping)

set(CMAKE_CXX_STANDARD 17)

add_executable(ts main.cpp stream.cpp dgram.cpp util.cpp)
