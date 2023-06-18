cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  libquill
  GIT_REPOSITORY https://github.com/odygrd/quill.git
  GIT_TAG        v3.0.2
)

set(QUILL_FMT_EXTERNAL ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(libquill)

