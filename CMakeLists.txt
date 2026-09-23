cmake_minimum_required(VERSION 3.16)
project(DiningPhilosophers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIRS})

add_executable(philosophers src/philosophers.cpp)
target_link_libraries(philosophers PRIVATE ${CURSES_LIBRARIES} pthread)