
add_library(utilities fft.cpp)
target_link_libraries(utilities fftw3 protobuffers devices)
target_include_directories(utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(test_pulse_detector test_pulse_detector.cpp)
target_link_libraries(test_pulse_detector utilities)
target_include_directories(test_pulse_detector PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
