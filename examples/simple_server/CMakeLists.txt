cmake_minimum_required(VERSION 3.3.2)
project(http_simple_server VERSION 1.0.0 DESCRIPTION "http simple server example")

include("${PROJECT_SOURCE_DIR}/third_party/cli11.cmake")
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} main.cpp )

target_link_libraries(${PROJECT_NAME} PUBLIC http_server::http_server CLI11::CLI11)
