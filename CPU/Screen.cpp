#include "Screen.hpp"
#include "PPU.hpp"
#include <SDL.h>
#include <Debug.hpp>
#include <tuple>
#include <Math.hpp>
#include <iostream>
#include <chrono>
#include <mutex>
#include <RAM.hpp>
#include <thread>

namespace Screen{
    SDL_Window* gbScreenWindow;
    SDL_Renderer* gbScreenRenderer;
    bool close = false, VBlank = false;
    std::tuple<uint8_t , uint8_t , uint8_t> colors[4];
    std::deque<int> fifo;
    void drawScreen();
    int scale = 4, prevScale = 0;
    int lastX = 0, lastY = 0;
    std::mutex fifoMutex;

    void gbScreenHandler(){
        /*colors[0] = std::make_tuple(15, 56, 15),
        colors[1] = std::make_tuple(48, 98, 48),
        colors[2] = std::make_tuple(139, 172, 15),
        colors[3] = std::make_tuple(155, 188, 15);*/

        colors[1] = std::make_tuple(255, 255, 255),
        colors[0] = std::make_tuple(170, 170, 170),
        colors[2] = std::make_tuple(85, 85, 85),
        colors[3] = std::make_tuple(0, 0, 0);


        fifo.clear();

        gbScreenWindow = SDL_CreateWindow("GBer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 160*scale, 144*scale, SDL_WINDOW_OPENGL);
        gbScreenRenderer = SDL_CreateRenderer(gbScreenWindow, -1, SDL_RENDERER_ACCELERATED);
        if(gbScreenRenderer == nullptr || gbScreenWindow == nullptr){
            Debug::emuLog(SDL_GetError(), Debug::ERR);
            return;
        }

        while(!close){
            if(prevScale != scale){
                prevScale = scale;
                SDL_SetWindowSize(gbScreenWindow, 160*scale, 144*scale);
            }
            auto beginning = std::chrono::steady_clock::now();
            SDL_Event evnt;
            while(SDL_PollEvent(&evnt)){
                switch(evnt.type){
                    case SDL_QUIT: close = true; break;
                    case SDL_KEYDOWN:
                        switch(evnt.key.keysym.sym){
                            case SDLK_z:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::A_BT);
                                break;
                            case SDLK_x:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::B_BT);
                                break;
                            case SDLK_a:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::SELECT);
                                break;
                            case SDLK_s:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::START);
                                break;
                            case SDLK_UP:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::UP);
                                break;
                            case SDLK_DOWN:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::DOWN);
                                break;
                            case SDLK_LEFT:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::LEFT);
                                break;
                            case SDLK_RIGHT:
                                RAM::RAM[0xFF00] &= ~(0xC0 | KeyBitMap::RIGHT);
                                break;
                            default: break;
                        }
                        break;
                    case SDL_KEYUP:
                        switch(evnt.key.keysym.sym){
                            case SDLK_z:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::A_BT);
                                break;
                            case SDLK_x:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::B_BT);
                                break;
                            case SDLK_a:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::SELECT);
                                break;
                            case SDLK_s:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::START);
                                break;
                            case SDLK_UP:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::UP);
                                break;
                            case SDLK_DOWN:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::DOWN);
                                break;
                            case SDLK_LEFT:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::LEFT);
                                break;
                            case SDLK_RIGHT:
                                RAM::RAM[0xFF00] |= (0xC0 | KeyBitMap::RIGHT);
                                break;
                            default: break;
                        }


                    default: break;
                }
            }

            SDL_SetRenderDrawColor(gbScreenRenderer, 202, 220, 159, 255);
            SDL_RenderClear(gbScreenRenderer);
            drawScreen();
            SDL_RenderPresent(gbScreenRenderer);


            //auto duration = beginning - std::chrono::steady_clock::now();
            //SDL_Delay( (1000/59.7) - std::chrono::duration_cast<std::chrono::milliseconds>(duration).count());
        }

        if(Debug::closed){
            Debug::TerminateGraphics();
        }

        SDL_DestroyRenderer(gbScreenRenderer);
        SDL_DestroyWindow(gbScreenWindow);

        return;
    }

    void drawScreen(){
        //  Base tile map address
        int tileOffsetX, tileOffsetY;
        int pixelOffsetX, pixelOffsetY;
        //  Calculate scrollX / scrollY
        tileOffsetX = RAM::RAM[0xFF43] / 8;
        tileOffsetY = RAM::RAM[0xFF42] / 8;
        pixelOffsetX = RAM::RAM[0xFF42] % 8;
        pixelOffsetY = RAM::RAM[0xFF43] % 8;
        int addr = PPU::bgTileMap.lower, addrEnd = PPU::bgTileMap.higher;

        RAM::RAM[0xFF44] = 0;
        //  Render 20x18 tiles on screen
        for(int j = 0 ; j < 144; j++){

            if(PPU::intOAM) RAM::RAM[0xFF0F] |= 1;
            PPU::currentPpuMode = PPU::OAM;
            PPU::updateStat();

            // TODO: OAM things
            for(int i = 0; i < 20; i++){
                PPU::currentPpuMode = PPU::PIXTX;
                PPU::updateStat();

                //  Get tile id and the address its' tile data is stored in
                int tileID = RAM::RAM[addr + ((( (j/8)+tileOffsetY)*0x20 + i + tileOffsetX - 1) % (addrEnd - addr))];
                int dataAddr = (PPU::bgWindowTileData.lower & 0xF00F) | (tileID << 4);

                //  Get the k - Lines and l - pixels that make up a sprite and draw them
                //  Get 2 bytes making up the tile from ram
                int spriteLine = j % 8;
                unsigned int data1 = RAM::RAM[dataAddr + 2*spriteLine], data2 = RAM::RAM[dataAddr + 2*spriteLine + 1];
                for(int l = 0; l < 8; l++){
                    //  Get color associated with pixel
                    int num = ((  (data1 & (1 << (8-l))) + (data2 & (1 << (8-l))) ) >> (8-l) )+ 1;
                    //std::cout << Math::decHex(dataAddr) + ": " + Math::decHex(data1) + "," + Math::decHex(data2) << num <<  "\n";
                    //  Set Color
                    SDL_SetRenderDrawColor(gbScreenRenderer, std::get<0>(colors[num]), std::get<1>(colors[num]), std::get<2>(colors[num]), 255);

                    //  Render with scaling
                    for(int addScaleX = 0; addScaleX < scale; addScaleX++){
                        for(int addScaleY = 0; addScaleY < scale; addScaleY++){
                            SDL_RenderDrawPoint(gbScreenRenderer, ((i)*8 + l)*scale + addScaleX, (j)*scale + addScaleY);
                        }
                    }
                }
            }
            RAM::RAM[0xFF44] += 1;
            if(PPU::intHBL) RAM::RAM[0xFF0F] |= 1;
            PPU::currentPpuMode = PPU::HBLANK;
            PPU::updateStat();
            //  Wait HBlank period
            std::this_thread::sleep_for(std::chrono::duration<double, std::nano>(48.6));
        }
        //  Wait VBlank
        PPU::currentPpuMode = PPU::VBLANK;
        PPU::updateStat();
        if(PPU::intVBL) RAM::RAM[0xFF0F] |= 1;
        while(RAM::RAM[0xFF44] != 153){
            std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
            RAM::RAM[0xFF44] += 1;
        }
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(1));
    }

    /*void drawScreen(){
        fifoMutex.lock();
        if(fifo.size() >= 8 && fifo.size() <= 16){
            int clr = fifo.front();
            fifo.pop_front();

            SDL_SetRenderDrawColor(gbScreenRenderer, std::get<0>(colors[clr]), std::get<1>(colors[clr]), std::get<2>(colors[clr]), 255 );
            for(int addScaleX = 0; addScaleX < scale; addScaleX++){
                for(int addScaleY = 0; addScaleY < scale; addScaleY++){
                    SDL_RenderDrawPoint(gbScreenRenderer, lastX*scale + addScaleX, (RAM::RAM[0xFF44]*scale + addScaleY));
                }
            }
            lastX = (lastX >= 159) ? 0 : lastX + 1;
        }
        fifoMutex.unlock();
    }*/
}