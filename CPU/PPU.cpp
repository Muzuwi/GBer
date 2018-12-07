#include <thread>
#include <vector>
#include <Debug.hpp>
#include <Math.hpp>
#include <iostream>
#include "../Memory/RAM.hpp"
#include "PPU.hpp"
#include "Screen.hpp"
namespace PPU {
    PPU_MODE currentPpuMode = OAM;
    AddrRange windowTileMap, bgTileMap, bgWindowTileData;
    int64_t cyclesSinceModeSwitch = 0;
    int pixelTransferFinishedFetches = 0, vblankLines = 0;

    bool intLYC, intOAM, intVBL, intHBL;
    bool LYC_eq;


    //  Handle PPU behavior
    //  Each cycle through all the ppu modes means one line drawn
    /*void handlePPU(int64_t cycles){
        //std::cout << cycles << "\n";
        bool doneProcessing = false;
        cyclesSinceModeSwitch += cycles;
        updateVariables();
        //std::cout << cyclesSinceModeSwitch << "\n";
        while(cyclesSinceModeSwitch > 140){
            doneProcessing = true;
            //  OAM behavior
            if(currentPpuMode == OAM){
                //  TODO: Sprite fifo
            }

            //  Switch to pix_tx
            if(currentPpuMode == OAM && cyclesSinceModeSwitch >= 20){
                //std::cout << "pix_tx\n";
                currentPpuMode = PIXTX;
                cyclesSinceModeSwitch -= 20;
                updateStat();
                doneProcessing = false;
            }

            //  Pixel transfer
            if(currentPpuMode == PIXTX && pixelTransferFinishedFetches <= 20){
                while(!Screen::fifoMutex.try_lock()){
                    std::this_thread::sleep_for(std::chrono::duration<double, std::nano>(1.0));
                }
                int fifoFreeFetches = (16 - Screen::fifo.size()) / 8;
                int cyclesPossibleFetches = (int)(cyclesSinceModeSwitch / 8);
                int fetchesLeftInLine = 20 - pixelTransferFinishedFetches;
                int fetchNum = std::min(std::min(fifoFreeFetches, fetchesLeftInLine), cyclesPossibleFetches);

                //std::cout << std::min(std::min(count, possibleFetches), 20) << "\n";
                for(int i = 0; i < fetchNum; i++){
                    int tileOffsetX = RAM::RAM[0xFF43] / 8,
                            tileOffsetY = RAM::RAM[0xFF42] / 8 ,
                            pixelOffsetX = RAM::RAM[0xFF42] % 8 ,
                            pixelOffsetY = RAM::RAM[0xFF43] % 8;

                    int addr = bgTileMap.lower, addrEnd = bgTileMap.higher;
                    int tileID = RAM::RAM[addr + ((( (RAM::RAM[0xFF44] / 8) + tileOffsetY - 1)*0x20 + pixelTransferFinishedFetches + tileOffsetX) % ((addrEnd+1) - addr))];
                    int dataAddr = (bgWindowTileData.lower & 0xF00F) | (tileID << 4);


                    unsigned int data1 = RAM::RAM[dataAddr], data2 = RAM::RAM[dataAddr + 1];
                    //std::cout << Math::decHex(dataAddr) + ": " + Math::decHex(data1) + "," + Math::decHex(data2) << " " << Math::decHex(tileID) << "\n";
                    for(int l = 0; l < 8; l++){
                        //  Get color associated with pixel
                        int num = ((  (data1 & (1 << (8-l))) + (data2 & (1 << (8-l))) ) >> (8-l) )+ 1;

                        int test = Screen::fifo.size();
                        if(test + 1 > 16){
                            std::cout << "#########################################\n";
                            std::cout << "Fifo error/overflow!\n";
                            std::cout << "fifo size @ crash: " << test << "\n";
                            std::cout << "fifo size: " << Screen::fifo.size() << "\n";
                            std::cout << pixelTransferFinishedFetches << " " << cyclesSinceModeSwitch << "\n";
                            std::cout << "calculated fetches " << fetchNum << "\n";
                            std::cout << "cyclePossible " << cyclesPossibleFetches << "\n";
                            std::cout << "fetchesleftinline " << fetchesLeftInLine << "\n";
                            std::cout << "fifoFreeFetches " << fifoFreeFetches << "\n";
                            std::cout << "ppu mode " << currentPpuMode << "\n";
                            assert(false);
                        }
                        Screen::fifo.push_back(num);
                        //std::cout << "push";
                    }



                    cyclesSinceModeSwitch -= 8;
                    pixelTransferFinishedFetches++;
                }
                Screen::fifoMutex.unlock();
            }

            //  Switch to HBlank
            if(currentPpuMode == PIXTX && cyclesSinceModeSwitch >= 43 && pixelTransferFinishedFetches == 20){
                //std::cout << "sw_pixtx\n";
                pixelTransferFinishedFetches = 0;
                currentPpuMode = HBLANK;
                cyclesSinceModeSwitch -= 43;
                updateStat();
                doneProcessing = false;
            }

            //  Switch from HBlank to VBlank/OAM
            if(currentPpuMode == HBLANK && cyclesSinceModeSwitch >= 51){
                //  Increase LY
                //std::cout << "hblank_sw\n";
                RAM::RAM[0xFF44] += 1;
                cyclesSinceModeSwitch -= 51;
                currentPpuMode = (RAM::RAM[0xFF44] >= 0x90) ? VBLANK : OAM;
                updateStat();
                doneProcessing = false;
            }

            if(currentPpuMode == VBLANK && vblankLines < 10 && cyclesSinceModeSwitch >= 114){
                //std::cout << "vblank";
                vblankLines++;
                RAM::RAM[0xFF44] += 1;
                cyclesSinceModeSwitch -= 114;
                updateStat();
                doneProcessing = false;
            }

            //  Leave VBlank
            if(currentPpuMode == VBLANK && vblankLines == 10){
                //std::cout << "vblank_sw\n";
                RAM::RAM[0xFF44] = 0;
                vblankLines = 0;
                currentPpuMode = OAM;
                Screen::VBlank = true;
                //Screen::VBlank = true;
                updateStat();
                doneProcessing = false;
            }

            //std::cout << doneProcessing;
        }
    }*/

    void updateVariables(){
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

    void updateStat(){
        int temp = RAM::RAM[0xFF41];
        temp &= 0xFC;
        temp |= currentPpuMode;
        RAM::RAM[0xFF41] = temp;
    }

}