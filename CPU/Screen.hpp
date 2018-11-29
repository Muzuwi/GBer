#pragma once
#include <SDL.h>
#include <vector>
#include <deque>

namespace Screen{
    extern SDL_Window* gbScreenWindow;
    extern SDL_Renderer* gbScreenRenderer;
    extern bool close, VBlank;
    extern std::deque<short> fifo;
    extern int lastX, lastY;
    void gbScreenHandler();
    void displayFromFifo();
}