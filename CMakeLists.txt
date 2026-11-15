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

add_executable(${PROJECT_NAME} src/main.cpp)
# add_compile_options(-fsanitize=address -fsanitize=undefined)
# add_link_options(-fsanitize=address -fsanitize=undefined)
target_link_libraries(${PROJECT_NAME} PRIVATE parlay)
