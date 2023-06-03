
add_executable(multi_sdr_server main.cpp)
target_link_libraries(multi_sdr_server fftw3 devices)
target_include_directories(multi_sdr_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS multi_sdr_server)
