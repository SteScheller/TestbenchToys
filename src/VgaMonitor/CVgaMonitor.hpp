#pragma once

#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <memory>
#include <vector>

#include <SDL.h>

class CVgaMonitor
{
    public:
        // types
        enum class Mode
        {
            VGA_640x480_60Hz
        };

        enum class ColorDepth
        {
            RGB_3BitPerColor
        };

        // methods
        CVgaMonitor() = default;
        ~CVgaMonitor();

        bool setup(Mode mode, ColorDepth depth);
        bool setup()
        {
            return setup(Mode::VGA_640x480_60Hz, ColorDepth::RGB_3BitPerColor);
        }

        void setShowTimingInfo(bool showTimingInfo);
        void setTimingTolerance(double tolerance);

        void eval(bool hSync, bool vSync, uint8_t red, uint8_t green, uint8_t blue,
                std::chrono::nanoseconds elapsed);

        bool hasQuitEvent();

    private:
        // types
        enum class State
        {
            HORIZONTAL_SYNC, VERTICAL_SYNC, LINE, OUT_OF_SYNC
        };

        using nanosec = std::chrono::nanoseconds;

        struct Pixel
        {
                uint8_t b;
                uint8_t g;
                uint8_t r;
                uint8_t padding;
        } __attribute__((__packed__));

        enum class TimingInfoBits : uint8_t
        {
            SYNC_BLANKING = 0,
            RGB_BLANKING = 1,
            SYNC_BACK_PORCH = 2,
            RGB_BACK_PORCH = 3,
            SYNC_ACTIVE_AREA = 4,
            SYNC_FRONT_PORCH = 5,
            RGB_FRONT_PORCH = 6
        };
        using TimingInfoBitfield = uint8_t;

        // methods
        void setupMode_VGA_640x480_60Hz();
        static TimingInfoBitfield checkSignalTiming(
            bool sync, bool isBlack, nanosec t, nanosec syncPulse, nanosec backPorch,
            nanosec visibleArea, nanosec frontPorch, double tolerance);
        void showTimingInfo(TimingInfoBitfield hTimingInfo, TimingInfoBitfield vTimingInfo);

        // members
        Mode m_mode { Mode::VGA_640x480_60Hz };
        ColorDepth m_depth { ColorDepth::RGB_3BitPerColor };
        State m_state { State::OUT_OF_SYNC };
        nanosec m_pixel { 0 };
        nanosec m_hSyncPulse { 0 };
        nanosec m_hBackPorch { 0 };
        nanosec m_hVisibleArea { 0 };
        nanosec m_hFrontPorch { 0 };
        nanosec m_line { 0 };
        nanosec m_vSyncPulse { 0 };
        nanosec m_vBackPorch { 0 };
        nanosec m_vVisibleArea { 0 };
        nanosec m_vFrontPorch { 0 };
        nanosec m_frame { 0 };
        double m_tolerance { 0.005 };
        size_t m_numPixels { 0 };
        size_t m_winWidth { 0 };
        size_t m_winHeight { 0 };
        uint8_t m_colorBitOffset { 0 };

        bool m_showTimingInfo { false };

        using windowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
        windowPtr m_window { nullptr, SDL_DestroyWindow };
        using rendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
        rendererPtr m_renderer { nullptr, SDL_DestroyRenderer };
        using texturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
        texturePtr m_texture { nullptr, SDL_DestroyTexture };

        std::vector<Pixel> m_buffer;
};

