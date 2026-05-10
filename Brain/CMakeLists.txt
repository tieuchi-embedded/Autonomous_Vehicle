cmake_minimum_required(VERSION 3.22)
project(bfmc_brain C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(libipc)
add_subdirectory(test)
add_subdirectory(perception/state)
add_subdirectory(perception/camera_sim)
add_subdirectory(perception/lane)
add_subdirectory(io/serial_bridge)
add_subdirectory(planning)
add_subdirectory(control)
add_subdirectory(control/manual_control)
