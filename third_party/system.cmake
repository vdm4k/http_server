cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  system
  GIT_REPOSITORY https://github.com/vdm4k/system.git
  GIT_TAG        origin/main
)

FetchContent_MakeAvailable(system)

