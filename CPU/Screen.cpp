#include "Screen.hpp"
#include "PPU.hpp"
#include <SDL.h>
#include <Debug.hpp>
#include <tuple>
#include <Math.hpp>
#include <iostream>
#include <chrono>

namespace Screen{
    SDL_Window* gbScreenWindow;
    SDL_Renderer* gbScreenRenderer;
    bool close = false, VBlank = false;
    std::tuple<uint8_t , uint8_t , uint8_t> colors[4];
    std::deque<short> fifo = {0};
    int lastX = 0, lastY = 0;

    void gbScreenHandler(){
        colors[0] = std::make_tuple(15, 56, 15),
        colors[1] = std::make_tuple(48, 98, 48),
        colors[2] = std::make_tuple(139, 172, 15),
        colors[3] = std::make_tuple(155, 188, 15);
        //fifo.resize(16);
        fifo.clear();

        gbScreenWindow = SDL_CreateWindow("GBer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160, 144, SDL_WINDOW_OPENGL);
        gbScreenRenderer = SDL_CreateRenderer(gbScreenWindow, -1, SDL_RENDERER_ACCELERATED);
        if(gbScreenRenderer == nullptr || gbScreenWindow == nullptr){
            Debug::emuLog(SDL_GetError(), Debug::ERR);
            return;
        }

        while(!close){
            auto beginning = std::chrono::steady_clock::now();
            SDL_Event evnt;
            while(SDL_PollEvent(&evnt)){
                switch(evnt.type){
                    case SDL_QUIT: close = true; break;
                    default: break;
                }
            }

            displayFromFifo();
            SDL_RenderPresent(gbScreenRenderer);
            if(VBlank){
                SDL_SetRenderDrawColor(gbScreenRenderer, 202, 220, 159, 255);
                SDL_RenderClear(gbScreenRenderer);
                VBlank = false;
            }
            auto duration = beginning - std::chrono::steady_clock::now();
            SDL_Delay( (1000/60) - std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }
    }

    void displayFromFifo(){
        if(fifo.size() > 16){
            Debug::emuLog("FIFO Overflow", Debug::LEVEL::ERR);
        }
        if(fifo.size() > 8){
            while(fifo.size() > 8 && fifo.size() <= 16){
                //std::cout << "Pixel drawn" << fifo.front() << " -> (" << lastX << ", " << PPU::LY << ")\n";
                if(fifo.front() >= 0 && fifo.front() <= 3){
                    SDL_SetRenderDrawColor(gbScreenRenderer, std::get<0>(colors[fifo.front()]), std::get<1>(colors[fifo.front()]), std::get<2>(colors[fifo.front()]), 255);
                }
                SDL_RenderDrawPoint(gbScreenRenderer, lastX, PPU::LY);
                lastX = (lastX >= 159 ) ? 0 : lastX + 1;
                //std::cout << lastX << " " << PPU::LY << "\n";
                if(fifo.size() > 0){
                    fifo.pop_front();
                }
            }
        }
    }

}