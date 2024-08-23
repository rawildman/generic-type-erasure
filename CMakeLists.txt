cmake_minimum_required(VERSION 3.5)

project(GenericTypeErasure LANGUAGES CXX VERSION 0.0.1)

add_subdirectory(src)

set(WARNINGS -Wall -Wextra -Wshadow -pedantic)

if(BUILD_TESTS)
  Include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.7.0
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

  add_subdirectory(tests)
endif()

