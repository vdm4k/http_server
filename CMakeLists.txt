cmake_minimum_required(VERSION 3.14.0)
project(http_server VERSION 0.9.0 DESCRIPTION "http server library")

# core h files
set(H_FILES
    include/http_server/request.h
    include/http_server/response.h
    include/http_server/http_server.h
    include/http_server/request_handler.h
    include/http_server/config.h
    include/http_server/private/request_parser.h    
    include/http_server/private/thread.h
    include/http_server/private/connection.h
    include/http_server/private/client_settings.h
)

# core cpp files
set(CPP_FILES
    source/request.cpp
    source/response.cpp
    source/http_server.cpp
    source/private/request_parser.cpp
    source/private/thread.cpp
)

include("${PROJECT_SOURCE_DIR}/third_party/http_client.cmake")
include("${PROJECT_SOURCE_DIR}/third_party/quill.cmake")
include("${PROJECT_SOURCE_DIR}/third_party/system.cmake")
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} STATIC ${CPP_FILES} ${H_FILES})
add_library(http_server::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_compile_options(${PROJECT_NAME} PRIVATE  -Wall -Wextra -Wpedantic -Wno-vla -Wno-unused-local-typedefs)

target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC
    $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/${PROJECT_NAME}>)

#sanitizer
option(WITH_SANITIZER "Enable address sanitizer" OFF)
set(EXPORT_COMPILE_COMMANDS TRUE)

if(WITH_SANITIZER)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE True)
    target_compile_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
    target_link_options(${PROJECT_NAME} INTERFACE -fsanitize=address)
endif()

#examples
option(WITH_EXAMPLES "Build examples" OFF)
if(WITH_EXAMPLES)
    add_subdirectory(examples)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads http_client::http_client quill::quill system::system)
