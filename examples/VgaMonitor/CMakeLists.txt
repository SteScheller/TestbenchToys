cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_CXX_COMPILER "g++")

project(VgaMonitorExample)

# testbench program
file(GLOB source_files "*.cpp")
add_executable(${PROJECT_NAME} ${source_files})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-sign-compare -std=c++17")
add_subdirectory(../../src/VgaMonitor ${PROJECT_BINARY_DIR}/VgaMonitor)
target_link_libraries(${PROJECT_NAME} PRIVATE vgamonitor)

# simulated vga controller
find_package(verilator HINTS $ENV{VERILATOR_ROOT} ${VERILATOR_ROOT})
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()
file(GLOB hdl_v_files "../../resources/VgaTestbench/*.v")
file(GLOB hdl_sv_files "../../resources/VgaTestbench/*.sv")
verilate(${PROJECT_NAME}
    SOURCES ${hdl_v_files} ${hdl_sv_files}
    TOP_MODULE VGA_TLM
    VERILATOR_ARGS -Wall -Wno-DECLFILENAME
    )
