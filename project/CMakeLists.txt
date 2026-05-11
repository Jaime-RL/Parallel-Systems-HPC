cmake_minimum_required(VERSION 3.10)
project(ParallelGOL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)
find_package(Curses REQUIRED)

include_directories(src ${CURSES_INCLUDE_DIR})

add_executable(GameOfLife 
    src/main.cpp 
    src/gol_seq.cpp 
    src/gol_mult.cpp
    src/tui.cpp
)

target_link_libraries(
    GameOfLife PRIVATE 
    OpenMP::OpenMP_CXX 
    ${CURSES_LIBRARIES}
)