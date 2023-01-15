
add_executable(radar_tx main_tx.cpp)
target_link_libraries(radar_tx devices)
target_include_directories(radar_tx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS radar_tx RUNTIME DESTINATION bin)


add_executable(radar_rx main_rx.cpp)
target_link_libraries(radar_rx devices)
target_include_directories(radar_rx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS radar_rx RUNTIME DESTINATION bin)
