#include <cstdlib>
#include <cstdint>

class CVgaMonitor
{
    public:
        // types
        static constexpr size_t DEFAULT_WIDTH = 640;
        static constexpr size_t DEFAULT_HEIGHT = 480;
        static constexpr size_t DEFAULT_COLOR_RESOLUTION = 3;

        // methods
        CVgaMonitor() = default;
        bool Setup(size_t width, size_t height, size_t bitsPerColor);
        bool Setup(){ return Setup(DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_COLOR_RESOLUTION); };

        void Evaluate(bool hSync, bool vSync, uint16_t red, uint16_t green, uint16_t blue);
};
