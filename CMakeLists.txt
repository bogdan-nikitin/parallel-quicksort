cmake_minimum_required(VERSION 4.2)
project(quicksort LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS On)

include(FetchContent)
FetchContent_Declare(parlaylib
    GIT_REPOSITORY  https://github.com/cmuparlay/parlaylib.git
    GIT_TAG         master
)
FetchContent_MakeAvailable(parlaylib)  
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_subdirectory(test)
