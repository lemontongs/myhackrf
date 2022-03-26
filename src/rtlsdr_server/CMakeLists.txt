
add_executable(rtlsdr_server main.cpp)
target_link_libraries(rtlsdr_server fftw3 utilities devices)
target_include_directories(rtlsdr_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
