cmake_minimum_required(VERSION 3.19)

project(VgaMonitor)

find_package(verilator)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17")

file(GLOB source_files "*.cpp")
add_executable(${PROJECT_NAME} ${source_files})

file(GLOB hdl_v_files "../../resources/VgaTestbench/*.v")
file(GLOB hdl_sv_files "../../resources/VgaTestbench/*.sv")
verilate(${PROJECT_NAME}
    SOURCES ${hdl_v_files} ${hdl_sv_files}
    TOP_MODULE VGA_TLM
    PREFIX VGA
    VERILATOR_ARGS -Wall -Wno-DECLFILENAME -Wno-fatal)
