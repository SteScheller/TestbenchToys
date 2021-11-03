#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>
#include <string>

#include "CVgaMonitor.hpp"

#include <SDL.h>

using namespace std::chrono_literals;

bool CVgaMonitor::setup(Mode mode, ColorDepth depth)
{
    auto ok = true;

    m_mode = mode;
    m_depth = depth;

    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Hello World",
            "You have successfully compiled and linked an SDL2"
                    " program, congratulations.", NULL);

    switch (mode)
    {
        case Mode::VGA_640x480_60Hz:
            setupMode_VGA_640x480_60Hz();
            break;

        default:
            assert(false);
            break;
    }

    m_window = windowPtr { SDL_CreateWindow("Simulated VGA Monitor", SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED, m_winWidth, m_winHeight, SDL_WINDOW_SHOWN), SDL_DestroyWindow };
    if (!m_window)
    {
        std::cerr << "vga monitor window creation failed: %s\n" << SDL_GetError();
        ok = false;
    }

    m_renderer = rendererPtr { SDL_CreateRenderer(m_window.get(), -1,
            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer };
    if (!m_renderer)
    {
        std::cerr << "vga monitor renderer creation failed: %s\n" << SDL_GetError();
        ok = false;
    }

    uint32_t pixelFormat = 0;
    switch (depth)
    {
        case ColorDepth::RGB_3BitPerColor:
            pixelFormat = SDL_PIXELFORMAT_RGB888;
            break;

        default:
            assert(false);
            break;
    }
    m_texture = texturePtr { SDL_CreateTexture(m_renderer.get(), pixelFormat,
            SDL_TEXTUREACCESS_TARGET, m_winWidth, m_winHeight), SDL_DestroyTexture };
    if (!m_texture)
    {
        std::cerr << "vga monitor texture creation failed: %s\n" << SDL_GetError();
        ok = false;
    }

    return ok;
}

void CVgaMonitor::setupMode_VGA_640x480_60Hz()
{
    m_t = nanosec { 0 };
    m_pixel = nanosec { std::lround(1.0e9 / 25.175e6) };
    m_hSync = nanosec { 96 * m_pixel };
    m_hBackPorch = nanosec { 48 * m_pixel };
    m_hVisibleArea = nanosec { 640 * m_pixel };
    m_hFrontPorch = nanosec { 16 * m_pixel };
    m_line = nanosec { 800 * m_pixel };
    m_vSync = nanosec { 2 * m_line };
    m_vBackPorch = nanosec { 33 * m_line };
    m_vVisibleArea = nanosec { 480 * m_line };
    m_vFrontPorch = nanosec { 10 * m_line };
    m_frame = nanosec { 525 * m_line };
    m_winWidth = 640;
    m_winHeight = 480;
    m_numPixels = m_winWidth * m_winHeight;
}

void CVgaMonitor::eval(bool hSync, bool vSync, uint16_t red, uint16_t green, uint16_t blue,
        std::chrono::nanoseconds elapsed)
{
    std::string errorTxt;
    bool run = true;

    m_t += elapsed;

    while (run)
    {
        run = false;
        switch (m_state)
        {
            case State::OUT_OF_SYNC:
                if (m_hSyncLast && !hSync)
                {
                    m_t = 0ns;
                    m_state = State::HORIZONTAL_SYNC;
                }
                break;

            case State::HORIZONTAL_SYNC:
                if (m_t > m_hSync)
                {
                    m_state = State::LINE;
                    run = true;
                }
                else if (hSync)
                {
                    errorTxt += "horizontal synchronization error: sync pulse too short\n";
                }
                else if (red || green || blue)
                {
                    errorTxt += "horizontal blanking error: RGB != 0\n";
                }
                break;

            case State::LINE:
                break;

            case State::VERTICAL_SYNC:
                break;

            default:
                assert(false);
                break;

        }
    }

    m_hSyncLast = hSync;
    m_vSyncLast = vSync;
}
