cmake_minimum_required(VERSION 3.30)
project(MySemesterProject)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY http://github.com/fmtlib/fmt
        GIT_TAG 11.0.2
)

FetchContent_Declare(
        SFML
        GIT_REPOSITORY http://github.com/SFML/SFML.git
        GIT_TAG 3.0.x
        GIT_SHALLOW ON
        EXCLUDE_FROM_ALL
        SYSTEM
)

FetchContent_MakeAvailable(SFML fmt)

add_executable(MySemesterProject main.cpp)

target_link_libraries(MySemesterProject PRIVATE SFML::Graphics fmt::fmt)
