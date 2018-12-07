#pragma once
#include <SDL.h>
#include <vector>
#include <deque>
#include <mutex>

namespace Screen{
    extern SDL_Window* gbScreenWindow;
    extern SDL_Renderer* gbScreenRenderer;
    extern bool close, VBlank;
    extern std::deque<int> fifo;
    extern std::tuple<uint8_t , uint8_t , uint8_t> colors[4];
    extern int scale;
    extern std::mutex fifoMutex;
    void gbScreenHandler();

    enum KeyBitMap{
        RIGHT =  0x10 | 0x1,
        LEFT = 0x10 | 0x2,
        UP = 0x10 | 0x4,
        DOWN = 0x10 | 0x8,
        A_BT = 0x20 | 0x1,
        B_BT = 0x20 | 0x2,
        SELECT = 0x20 | 0x4,
        START = 0x20 | 0x8,
    };

}