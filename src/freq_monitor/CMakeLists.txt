
add_executable(freq_monitor main.cpp)
target_link_libraries(freq_monitor protobuf zmq protobuffers devices)
target_include_directories(freq_monitor PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS freq_monitor RUNTIME DESTINATION bin)
