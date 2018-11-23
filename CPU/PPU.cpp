#include <vector>
#include <Debug.hpp>
#include <Math.hpp>
#include <iostream>
#include "../Memory/RAM.hpp"
#include "PPU.hpp"
#include "Screen.hpp"
namespace PPU {
    PPU_MODE currentPpuMode = OAM;
    PPUControl LCDC;
    int cycleSinceModeSwitch = 0, LY = 0, SCX = 0, SCY = 0;
    int pixTxFinishedFetches = 0;
    int pushes = 0;

    AddrRange windowTileMap, bgTileMap, bgWindowTileData;

    bool idle = false;
    std::vector<OAMEntry> visibleSprites;

    void handlePPU(int cycles){
        updateVariables();


        cycleSinceModeSwitch += cycles;
        if(currentPpuMode == OAM && !idle){
            //   OAM things
            visibleSprites.clear();
            for(int i = 0xFE00; i <= 0xFE9C; i += 4){
                if(visibleSprites.size() == 10) break;
                OAMEntry temp;
                temp.posY = RAM::RAM[i];
                temp.posX = RAM::RAM[i + 1];
                temp.tileNo = RAM::RAM[i + 2];
                int attrib = RAM::RAM[i + 3];
                temp.priority = (attrib & 0x80);
                temp.flipY = (attrib & 0x40);
                temp.flipX = (attrib & 0x20);
                if(temp.posX != 0 && (LY + 16 >= temp.posY) && (LY + 16 < temp.posY + LCDC.objH) ){
                    visibleSprites.push_back(temp);
                }
            }
            idle = true;
        }

        if(currentPpuMode == OAM && cycleSinceModeSwitch >= 20){
            idle = false;
            //Debug::emuLog("Switching to pixel transfer");
            currentPpuMode = PIXTX;
            cycleSinceModeSwitch -= 20;
            updateSTAT();
        } 

        if(currentPpuMode == PIXTX && pixTxFinishedFetches < 19){
            int temp = cycleSinceModeSwitch;
            int count = (16 - Screen::fifo.size()) / 8;
            int possibleFetches = cycleSinceModeSwitch / 8;

            if(Screen::fifo.size() > 15) {
               // std::cout << Screen::fifo.size() << " " << count << "        " << possibleFetches << "\n";
            }

            for(int i = 0; i < std::min(count, possibleFetches); i++){
                //  Look up tile id
                int addr = bgTileMap.lower + pixTxFinishedFetches;
                int tileID = RAM::RAM[addr];
                //  Look up tile data
                if(bgWindowTileData.lower == 0x8800){

                } else {
                    int dataAddr = bgWindowTileData.lower + tileID*16 + (2*(LY % 8));
                    unsigned int data1 = RAM::RAM[dataAddr], data2 = RAM::RAM[dataAddr + 1];
                    for(int j = 7; j > 0; j--){
                        Screen::fifo.push_back(((data2 & (1 << j)) >> (j - 1)) | ((data1 & (1 << j)) >> (j)) );
                    }
                }
                pixTxFinishedFetches++;
            }
        }
        
        if(currentPpuMode == PIXTX && cycleSinceModeSwitch >= 43){
            pixTxFinishedFetches = 0;
            currentPpuMode = HBLANK;
            cycleSinceModeSwitch -= 43;
            updateSTAT();
        }

        if(currentPpuMode == HBLANK && cycleSinceModeSwitch >= 51){
            //  Increase LY
            RAM::RAM[0xFF44] += 1;
            cycleSinceModeSwitch -= cycles;
            currentPpuMode = (LY == 0x90) ? VBLANK : OAM;
            updateSTAT();
        }

        //  Leave VBlank
        if(currentPpuMode == VBLANK && cycleSinceModeSwitch >= 1140){
            RAM::RAM[0xFF44] = 0;
            cycleSinceModeSwitch -= 1140;
            currentPpuMode = OAM;
            Screen::VBlank = true;
            updateSTAT();
        }
    }

    void updateVariables(){
        LY = RAM::RAM[0xFF44];
        SCX = RAM::RAM[0xFF43];
        SCY = RAM::RAM[0xFF42];
        if((RAM::RAM[0xFF40] & 0x8)){
            bgTileMap.lower = 0x9C00;
            bgTileMap.higher = 0x9FFF;
        } else {
            bgTileMap.lower = 0x9800;
            bgTileMap.higher = 0x9BFF;
        }

        if((RAM::RAM[0xFF40] & 0x10)){
            bgWindowTileData.lower = 0x8000;
            bgWindowTileData.higher = 0x8FFF;
        } else {
            bgWindowTileData.lower = 0x8800;
            bgWindowTileData.higher = 0x97FF;
        }

        if((RAM::RAM[0xFF40] & 0x40)){
            windowTileMap.lower = 0x9C00;
            windowTileMap.higher = 0x9FFF;
        } else {
            windowTileMap.lower = 0x9800;
            windowTileMap.higher = 0x9BFF;
        }

    }


    inline void updateSTAT(){
        int temp = RAM::RAM[0xFF41];
        temp &= 0xFC;
        temp |= currentPpuMode;
        RAM::RAM[0xFF41] = temp;
    }
}