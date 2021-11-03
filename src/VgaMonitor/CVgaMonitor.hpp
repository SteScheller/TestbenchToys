#pragma once

#include <cstdlib>
#include <cstdint>
#include <chrono>
#include <memory>

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
        bool setup(Mode mode, ColorDepth depth);
        bool setup()
        {
            return setup(Mode::VGA_640x480_60Hz, ColorDepth::RGB_3BitPerColor);
        }

        void eval(bool hSync, bool vSync, uint16_t red, uint16_t green, uint16_t blue,
                std::chrono::nanoseconds elapsed);

    private:
        // types
        enum class State
        {
            HORIZONTAL_SYNC, VERTICAL_SYNC, LINE, OUT_OF_SYNC
        };

        using nanosec = std::chrono::nanoseconds;

        // methods
        void setupMode_VGA_640x480_60Hz();

        // members
        Mode m_mode { Mode::VGA_640x480_60Hz };
        ColorDepth m_depth { ColorDepth::RGB_3BitPerColor };
        State m_state { State::OUT_OF_SYNC };
        nanosec m_t { 0 };
        nanosec m_pixel { 0 };
        nanosec m_hSync { 0 };
        nanosec m_hBackPorch { 0 };
        nanosec m_hVisibleArea { 0 };
        nanosec m_hFrontPorch { 0 };
        nanosec m_line { 0 };
        nanosec m_vSync { 0 };
        nanosec m_vBackPorch { 0 };
        nanosec m_vVisibleArea { 0 };
        nanosec m_vFrontPorch { 0 };
        nanosec m_frame { 0 };
        size_t m_numPixels { 0 };
        size_t m_winWidth { 0 };
        size_t m_winHeight { 0 };
        bool m_hSyncLast { false };
        bool m_vSyncLast { false };

        using windowPtr = std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)>;
        windowPtr m_window { nullptr, SDL_DestroyWindow };
        using rendererPtr = std::unique_ptr<SDL_Renderer, decltype(&SDL_DestroyRenderer)>;
        rendererPtr m_renderer { nullptr, SDL_DestroyRenderer };
        using texturePtr = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>;
        texturePtr m_texture { nullptr, SDL_DestroyTexture };
};

