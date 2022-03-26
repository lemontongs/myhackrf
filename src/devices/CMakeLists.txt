
add_library(devices HackRFDevice.cpp RTLSDRDevice.cpp SDRReceiver.cpp)
target_link_libraries(devices protobuf zmq pthread utilities hackrf rtlsdr)
target_include_directories(devices PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_hackrf_cpp test_hackrf.cpp)
target_link_libraries(test_hackrf_cpp devices)

add_executable(test_rtlsdr_cpp test_rtl.cpp)
target_link_libraries(test_rtlsdr_cpp devices)

add_executable(test_sdr_receiver test_sdr_receiver.cpp)
target_link_libraries(test_sdr_receiver devices)
