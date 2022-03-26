
cmake_minimum_required(VERSION 3.13)

set(CMAKE_INSTALL_PREFIX /tmp/myhackrf/)

project(myHackRF)

add_subdirectory(src/protobuffers)
add_subdirectory(src/utilities)
add_subdirectory(src/devices)
add_subdirectory(src/sdr_server)
add_subdirectory(src/hackrf_simple_server)
add_subdirectory(src/freq_monitor)
