
add_executable(hackrf_simple_server Socket.cpp ServerSocket.cpp main.cpp)
target_link_libraries(hackrf_simple_server devices)
target_include_directories(hackrf_simple_server PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
