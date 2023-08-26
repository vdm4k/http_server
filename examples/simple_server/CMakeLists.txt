cmake_minimum_required(VERSION 3.3.2)
project(http_simple_server VERSION 1.0.0 DESCRIPTION "http simple server example")

include("${PROJECT_SOURCE_DIR}/third_party/cli11.cmake")
include("${PROJECT_SOURCE_DIR}/third_party/yaml-cpp.cmake")
find_package(Threads REQUIRED)

# core h files
set(H_FILES
    source/config.h
)

# core cpp files
set(CPP_FILES
    source/config.cpp
    source/main.cpp
)

add_executable(${PROJECT_NAME} ${CPP_FILES} ${H_FILES} )

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

target_link_libraries(${PROJECT_NAME} PUBLIC http_server::http_server CLI11::CLI11 yaml-cpp::yaml-cpp)
