#include <iostream>
#include <cassert>
#include <cmath>
#include <chrono>
#include <string>

#include "CVgaMonitor.hpp"

#include <SDL.h>

using namespace std::chrono_literals;

namespace
{
    double tolerance { 0.0075 };
}

bool CVgaMonitor::setup(Mode mode, ColorDepth depth)
{
    auto ok = true;

    m_mode = mode;
    m_depth = depth;

    m_th = nanosec { 0 };
    m_tv = nanosec { 0 };
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

void CVgaMonitor::eval(bool hSync, bool vSync, uint8_t red, uint8_t green, uint8_t blue,
        std::chrono::nanoseconds elapsed)
{
    static std::string errorTxt;

    m_th += elapsed;
    m_tv += elapsed;

    // frame starts on the negative edge of vsync
    if (m_vSyncLast && !vSync)
    {
        m_tv = 0ns;

        // display last frame
        SDL_UpdateTexture(m_texture.get(), NULL, m_buffer.data(), m_winWidth * sizeof(Pixel));
        SDL_RenderClear(m_renderer.get());
        SDL_RenderCopy(m_renderer.get(), m_texture.get(), NULL, NULL);
        SDL_RenderPresent(m_renderer.get());

        if (!errorTxt.empty())
        {
            SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Debug", errorTxt.c_str(),
                    m_window.get());
        }

        // delete error text from last frame
        errorTxt.clear();
    }

    {
        // check vsync signal
        if ((m_tv * (1.0 + tolerance)) < m_vSyncPulse)
        {
            // vsync should be low during blanking
            if (vSync)
            {
                //errorTxt += "vertical synchronization error: vsync high during blanking\n";
            }

            // colors should be off during blanking
            if (red || green || blue)
            {
                //errorTxt += "vertical blanking error: RGB != 0 during synchronization\n";
            }
        }
        else if ((m_tv * (1.0 + tolerance)) < (m_vSyncPulse + m_vBackPorch))
        {
            // vsync should be high during back porch
            if (!vSync)
            {
                //errorTxt += "vertical synchronization error: vsync low during back porch\n";
            }

            // colors should be off back porch
            if (red || green || blue)
            {
                //errorTxt += "vertical blanking error: RGB != 0 during back porch\n";
            }
        }
        else if (((m_tv * (1.0 - tolerance)) > (m_vSyncPulse + m_vBackPorch))
                && ((m_tv * (1.0 + tolerance)) < (m_vSyncPulse + m_vBackPorch + m_vVisibleArea)))
        {
            // vsync should be high in active area
            if (!vSync)
            {
                //errorTxt += "vertical synchronization error: vsync low in active area\n";
            }
        }
        else if (((m_tv * (1.0 - tolerance)) > (m_vSyncPulse + m_vBackPorch + m_vVisibleArea))
                && ((m_tv * (1.0 + tolerance)) < (m_frame - m_vFrontPorch)))
        {
            // vsync should be high during front porch
            if (!vSync)
            {
                //errorTxt += "vertical synchronization error: vsync low during front porch\n";
            }

            // colors should be off back porch
            if (red || green || blue)
            {
                //errorTxt += "vertical blanking error: RGB != 0 during front porch\n";
            }
        }
    }

    // line  starts on the negative edge of hsync
    if (m_hSyncLast && !hSync)
    {
        m_th = 0ns;
    }

    // check hsync signal
    {
        if ((m_th * (1.0 + tolerance)) < m_hSyncPulse)
        {
            // hsync should be low during blanking
            if (hSync)
            {
                //errorTxt += "horizontal synchronization error: hsync high during blanking\n";
            }

            // colors should be off during blanking
            if (red || green || blue)
            {
                //errorTxt += "horizontal blanking error: RGB != 0 during synchronization\n";
            }
        }
        else if ((m_th * (1.0 + tolerance)) < (m_hSyncPulse + m_hBackPorch))
        {
            // hsync should be high during back porch
            if (!hSync)
            {
                //errorTxt += "horizontal synchronization error: hsync low during back porch\n";
            }

            // colors should be off back porch
            if (red || green || blue)
            {
                //errorTxt += "horizontal blanking error: RGB != 0 during back porch\n";
            }
        }
        else if (((m_th * (1.0 - tolerance)) > (m_hSyncPulse + m_hBackPorch))
                && ((m_th * (1.0 + tolerance)) < (m_hSyncPulse + m_hBackPorch + m_hVisibleArea)))
        {
            // hsync should be high in active area
            if (!hSync)
            {
                //errorTxt += "horizontal synchronization error: hsync low in active area\n";
            }
        }
        else if (((m_th * (1.0 - tolerance)) > (m_hSyncPulse + m_hBackPorch + m_hVisibleArea))
                && ((m_th * (1.0 + tolerance)) < (m_frame - m_hFrontPorch)))
        {
            // hsync should be high during front porch
            if (!hSync)
            {
                //errorTxt += "horizontal synchronization error: hsync low during front porch\n";
            }

            // colors should be off back porch
            if (red || green || blue)
            {
                //errorTxt += "horizontal blanking error: RGB != 0 during front porch\n";
            }
        }
    }

    // color the current pixel
    {
        size_t x = m_winWidth;
        size_t y = m_winHeight;

        nanosec xt = m_th - m_hSyncPulse - m_hBackPorch;
        nanosec yt = m_tv - m_vSyncPulse - m_vBackPorch;
        if ((xt >= 0ns) && (yt >= 0ns))
        {
            x = floor(xt / m_pixel);
            y = floor(yt / m_line);
        }

        if ((x < m_winWidth) && (y < m_winHeight))
        {
            auto &pixel = m_buffer[y * m_winWidth + x];
            pixel.r = red << m_colorBitOffset;
            pixel.g = green << m_colorBitOffset;
            pixel.b = blue << m_colorBitOffset;
        }
    }

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
