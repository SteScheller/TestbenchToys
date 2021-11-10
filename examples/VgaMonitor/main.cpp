#include <cstdlib>
#include <iostream>
#include <chrono>
#include <cassert>

#include <verilated.h>

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
    Verilated::commandArgs(argc, argv);

    VVGA_top controller;
    vluint64_t time { 0 };

    CVgaMonitor monitor;

    assert(
            monitor.setup(CVgaMonitor::Mode::VGA_640x480_60Hz,
                    CVgaMonitor::ColorDepth::RGB_3BitPerColor));

    // Tick the clock until we are done
    while (!Verilated::gotFinish() && !monitor.hasQuitEvent())
    {
        bool clk = time % 2;

        controller.i_clk = clk;
        controller.eval();
        monitor.eval(controller.o_vgaHSync, controller.o_vgaVSync, getRed(controller),
                getGreen(controller), getBlue(controller), 20ns);

        ++time;
    }

    exit(EXIT_SUCCESS);
}
