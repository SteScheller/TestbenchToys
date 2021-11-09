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

    m_t = nanosec { 0 };
    switch (mode)
    {
        case Mode::VGA_640x480_60Hz:
            setupMode_VGA_640x480_60Hz();
            break;

        default:
            assert(false);
            break;
    }
    m_numPixels = m_winWidth * m_winHeight;
    m_buffer.resize(m_numPixels, { 0, 0, 0, 0 });

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

    switch (depth)
    {
        case ColorDepth::RGB_3BitPerColor:
            m_colorBitOffset = 5;
            break;

        default:
            assert(false);
            break;
    }
    uint32_t pixelFormat = SDL_PIXELFORMAT_RGB888;
    m_texture = texturePtr { SDL_CreateTexture(m_renderer.get(), pixelFormat,
            SDL_TEXTUREACCESS_STREAMING, m_winWidth, m_winHeight), SDL_DestroyTexture };
    if (!m_texture)
    {
        std::cerr << "vga monitor texture creation failed: %s\n" << SDL_GetError();
        ok = false;
    }
    else
    {
        std::cout << "texture has alpha: " << SDL_ISPIXELFORMAT_ALPHA(pixelFormat) << std::endl;
        std::cout << "texture bits per pixel: " << SDL_BITSPERPIXEL(pixelFormat) << std::endl;
        std::cout << "texture bytes per pixel: " << SDL_BYTESPERPIXEL(pixelFormat) << std::endl;
    }

    return ok;
}

void CVgaMonitor::setupMode_VGA_640x480_60Hz()
{
    m_pixel = nanosec { std::lround(1.0e9 / 25.175e6) };
    m_hSyncPulse = nanosec { 96 * m_pixel };
    m_hBackPorch = nanosec { 48 * m_pixel };
    m_hVisibleArea = nanosec { 640 * m_pixel };
    m_hFrontPorch = nanosec { 16 * m_pixel };
    m_line = nanosec { 800 * m_pixel };
    m_vSyncPulse = nanosec { 2 * m_line };
    m_vBackPorch = nanosec { 33 * m_line };
    m_vVisibleArea = nanosec { 480 * m_line };
    m_vFrontPorch = nanosec { 10 * m_line };
    m_frame = nanosec { 525 * m_line };
    m_winWidth = 640;
    m_winHeight = 480;
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
                if (m_vSyncLast && !vSync)
                {
                    m_t = 0ns;
                    m_state = State::VERTICAL_SYNC;
                }
                break;

            case State::VERTICAL_SYNC:
                if ((m_t >= m_vSyncPulse) && (m_hSyncLast && !hSync))
                {
                    m_state = State::HORIZONTAL_SYNC;
                    run = true;
                }
                else if (vSync)
                {
                    errorTxt += "vertical synchronization error: sync pulse too short\n";
                }
                else if (red || green || blue)
                {
                    errorTxt += "vertical blanking error: RGB != 0\n";
                }
                break;

            case State::HORIZONTAL_SYNC:
                break;

            case State::LINE:
                break;

            default:
                assert(false);
                break;

        }
    }

    SDL_UpdateTexture(m_texture.get(), NULL, m_buffer.data(), m_winWidth * sizeof(Pixel));
    SDL_RenderClear(m_renderer.get());
    SDL_RenderCopy(m_renderer.get(), m_texture.get(), NULL, NULL);
    SDL_RenderPresent(m_renderer.get());

    m_hSyncLast = hSync;
    m_vSyncLast = vSync;
}

bool CVgaMonitor::hasQuitEvent()
{
    auto shallQuit = false;

    SDL_Event e;
    if (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            shallQuit = true;
        }
    }

    return shallQuit;
}
