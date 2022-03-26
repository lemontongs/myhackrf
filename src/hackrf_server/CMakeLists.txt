
add_executable(hackrf_server main.cpp)
target_link_libraries(hackrf_server fftw3 devices)
target_include_directories(hackrf_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS hackrf_server)
