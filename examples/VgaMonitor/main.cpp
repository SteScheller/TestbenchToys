#include <cstdlib>
#include <iostream>
#include <chrono>

#include <verilated.h>
#include <verilated_vcd_c.h>

#include "CVgaMonitor.hpp"
#include "VVGA_top.h"

using namespace std::chrono_literals;

inline uint8_t getRed(const VVGA_top &c)
{
    return static_cast<uint8_t>((c.o_vgaR2 << 2) | (c.o_vgaR1 << 1) | c.o_vgaR0);
}

inline uint8_t getGreen(const VVGA_top &c)
{
    return static_cast<uint8_t>((c.o_vgaG2 << 2) | (c.o_vgaG1 << 1) | c.o_vgaG0);
}

inline uint8_t getBlue(const VVGA_top &c)
{
    return static_cast<uint8_t>((c.o_vgaB2 << 2) | (c.o_vgaB1 << 1) | c.o_vgaB0);
}

int main(int argc, char **argv)
{
    // initialize verilator variables
    VerilatedContext context;
    context.commandArgs(argc, argv);

    // set up the VGA controller
    VVGA_top controller;
    controller.i_clk = 0;

    // set up the simulated vga monitor
    CVgaMonitor monitor;
    auto monitorSetupSuccessful = monitor.setup(
        CVgaMonitor::Mode::VGA_640x480_60Hz, CVgaMonitor::ColorDepth::RGB_3BitPerColor);
    if (!monitorSetupSuccessful)
    {
        std::cerr << "Monitor setup failed" << std::endl;
        return EXIT_FAILURE;
    }
    monitor.setShowTimingInfo(true);
    monitor.setTimingTolerance(0.0075);

    // set up tracing
    context.traceEverOn(true);
    VerilatedVcdC tracer;
    controller.trace(&tracer, 0);
    tracer.open("vga_monitor_example.vcd");

    // Tick the clock until we are done
    while (!Verilated::gotFinish() && !monitor.hasQuitEvent())
    {
        bool clk = context.time() % 2;

        controller.i_clk = clk;
        controller.eval();
        monitor.eval(controller.o_vgaHSync, controller.o_vgaVSync, getRed(controller),
                getGreen(controller), getBlue(controller), 20ns);

        tracer.dump(context.time());

        context.timeInc(1);
    }

    controller.final();

    exit(EXIT_SUCCESS);
}
