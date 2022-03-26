
add_executable(sdr_server main.cpp)
target_link_libraries(sdr_server fftw3 devices)
target_include_directories(sdr_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS sdr_server)
