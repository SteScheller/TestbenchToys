#include <iostream>
#include <cassert>
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
    static TimingInfoBitfield hTimingInfo { 0};
    static TimingInfoBitfield vTimingInfo { 0};
    static nanosec th{0};
    static nanosec tv{0};
    static bool hSyncLast { false };
    static bool vSyncLast { false };

    th += elapsed;
    tv += elapsed;
    bool isBlack = (red == 0) && (green == 0) && (blue == 0);

    // frame starts on the negative edge of vsync
    if (vSyncLast && !vSync)
    {
        tv = 0ns;

        // display last frame
        SDL_UpdateTexture(m_texture.get(), NULL, m_buffer.data(), m_winWidth * sizeof(Pixel));
        SDL_RenderClear(m_renderer.get());
        SDL_RenderCopy(m_renderer.get(), m_texture.get(), NULL, NULL);
        SDL_RenderPresent(m_renderer.get());

        if (m_showTimingInfo)
        {
            // todo
        }

        // reset timing info bitfield
        hTimingInfo = 0;
        vTimingInfo = 0;
    }
    vTimingInfo |= checkSignalTiming(
        vSync, isBlack, tv, m_vSyncPulse, m_vBackPorch, m_vVisibleArea, m_vFrontPorch, m_tolerance);


    // line starts on the negative edge of hsync
    if (hSyncLast && !hSync)
    {
        th = 0ns;
    }
    hTimingInfo |= checkSignalTiming(
        hSync, isBlack, th, m_hSyncPulse, m_hBackPorch, m_hVisibleArea, m_hFrontPorch, m_tolerance);

    // color the current pixel
    {
        size_t x = m_winWidth;
        size_t y = m_winHeight;

        nanosec xt = th - m_hSyncPulse - m_hBackPorch;
        nanosec yt = tv - m_vSyncPulse - m_vBackPorch;
        if ((xt >= 0ns) && (yt >= 0ns) && (m_pixel > 0ns) && (m_line > 0ns))
        {
            x = static_cast<size_t>(xt / m_pixel);
            y = static_cast<size_t>(yt / m_line);
        }

        if ((x < m_winWidth) && (y < m_winHeight))
        {
            auto &pixel = m_buffer[y * m_winWidth + x];
            pixel.r = red << m_colorBitOffset;
            pixel.g = green << m_colorBitOffset;
            pixel.b = blue << m_colorBitOffset;
        }
    }

    hSyncLast = hSync;
    vSyncLast = vSync;
}

CVgaMonitor::TimingInfoBitfield CVgaMonitor::checkSignalTiming(
    bool sync, 
    bool isBlack, 
    nanosec t, 
    nanosec syncPulse,
    nanosec backPorch,
    nanosec visibleArea,
    nanosec frontPorch,
    double tolerance)
{
    TimingInfoBitfield timingInfo { 0 };
    if ((t * (1.0 + tolerance)) < syncPulse)
    {
        // vsync should be low during blanking
        if (sync) timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::SYNC_BLANKING));

        // colors should be off during blanking
        if (isBlack) timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::RGB_BLANKING));
    }
    else if ((t * (1.0 + tolerance)) < (syncPulse + backPorch))
    {
        // vsync should be high during back porch
        if (!sync) timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::SYNC_BACK_PORCH));

        // colors should be off during back porch
        if (isBlack)
            timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::RGB_BACK_PORCH));
    }
    else if (((t * (1.0 - tolerance)) > (syncPulse + backPorch))
            && ((t * (1.0 + tolerance)) < (syncPulse + backPorch + visibleArea)))
    {
        // vsync should be high in active area
        if (!sync)
            timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::SYNC_ACTIVE_AREA));
    }
    else if (((t * (1.0 - tolerance)) > (syncPulse + backPorch + visibleArea))
            && ((t * (1.0 + tolerance)) < (syncPulse + backPorch + visibleArea + frontPorch)))
    {
        // vsync should be high during front porch
        if (!sync)
            timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::SYNC_FRONT_PORCH));

        // colors should be off during front porch
        if (isBlack) timingInfo |= (1 << static_cast<uint8_t>(TimingInfoBits::RGB_FRONT_PORCH));
    }

    return timingInfo;
}

bool CVgaMonitor::hasQuitEvent()
{
    auto shallQuit = false;

    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)
        {
            shallQuit = true;
        }
    }

    return shallQuit;
}

void CVgaMonitor::setShowTimingInfo(bool showTimingInfo)
{
    m_showTimingInfo = showTimingInfo;
}

void CVgaMonitor::setTimingTolerance(double tolerance)
{
    m_tolerance = tolerance;
}