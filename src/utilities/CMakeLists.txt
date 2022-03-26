
add_library(utilities fft.cpp)
target_link_libraries(utilities protobuffers devices)
target_include_directories(utilities PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
