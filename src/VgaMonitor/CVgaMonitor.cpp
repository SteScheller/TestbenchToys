#include "CVgaMonitor.hpp"

#include <SDL.h>

bool CVgaMonitor::setup(size_t width, size_t height, size_t bitsPerColor)
{

	SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_INFORMATION,
	              "Hello World",
	              "You have successfully compiled and linked an SDL2"
	              " program, congratulations.", NULL );

    return true;
}

void CVgaMonitor::eval(bool hSync, bool vSync, uint16_t red, uint16_t green, uint16_t blue)
{
}
