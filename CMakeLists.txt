cmake_minimum_required(VERSION 3.8)

project(ujind VERSION 0.1)

add_executable(${PROJECT_NAME} main.c broadcast.c)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_link_libraries(${PROJECT_NAME} mosquitto curl pthread m dl)

add_executable(maplay_simple ma_main.c)
target_link_libraries(maplay_simple pthread m dl)

install(TARGETS ${PROJECT_NAME} maplay_simple DESTINATION bin)

