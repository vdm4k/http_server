cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  yaml-cpp
  GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
  GIT_TAG        yaml-cpp-0.7.0
)

get_property(
    compile_options
    DIRECTORY
    PROPERTY COMPILE_OPTIONS
)

set_property(
    DIRECTORY
    APPEND
    PROPERTY COMPILE_OPTIONS -Wno-shadow
)

FetchContent_MakeAvailable(yaml-cpp)

set_property(
    DIRECTORY
    PROPERTY COMPILE_OPTIONS ${compile_options}
)

unset(compile_options)


