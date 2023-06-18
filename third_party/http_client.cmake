cmake_minimum_required(VERSION 3.14.0)
include(FetchContent)
FetchContent_Declare(
  http_client
  GIT_REPOSITORY https://github.com/vdm4k/http_client
  GIT_TAG        origin/main
)

FetchContent_MakeAvailable(http_client)
