cmake_minimum_required(VERSION 3.8)

project(hello VERSION 0.1)

add_executable(${PROJECT_NAME} main.c)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)
