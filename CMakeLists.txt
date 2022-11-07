cmake_minimum_required(VERSION 3.8)

project(ujind VERSION 0.1)

add_executable(${PROJECT_NAME} main.c lte_at.c mcu_cmd.c cJSON.c serial.c serial_termios2.c conf.c toml.c logger.c)
set_target_properties(${PROJECT_NAME} PROPERTIES VERSION ${PROJECT_VERSION})
target_link_libraries(${PROJECT_NAME} mosquitto curl pthread m dl)

install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin)
install(FILES ${PROJECT_NAME} scripts/ujind.conf DESTINATION /etc)
