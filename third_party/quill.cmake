cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  libquill
  GIT_REPOSITORY https://github.com/odygrd/quill.git
  GIT_TAG        v3.0.2
)

get_property(
    compile_options
    DIRECTORY
    PROPERTY COMPILE_OPTIONS
)

set_property(
    DIRECTORY
    APPEND
    PROPERTY COMPILE_OPTIONS -Wno-gnu-zero-variadic-macro-arguments
)

set(QUILL_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libquill)

set_property(
    DIRECTORY
    PROPERTY COMPILE_OPTIONS ${compile_options}
)

unset(compile_options)

