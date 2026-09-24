cmake_minimum_required(VERSION 3.15)
project(RouletteSim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(roulette_sim
    src/main.cpp
    src/wheel.cpp
    src/strategies.cpp
    src/simulation.cpp
    src/statistics.cpp
    src/csv_writer.cpp
)

target_include_directories(roulette_sim PRIVATE src)

if (MSVC)
    target_compile_options(roulette_sim PRIVATE /W4)
else()
    target_compile_options(roulette_sim PRIVATE -Wall -Wextra -Wpedantic)
endif()
