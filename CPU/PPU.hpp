#pragma once
namespace PPU {
    extern int cycleSinceModeSwitch, LY, SCX, SCY;
    extern bool intLYC;
    extern bool intOAM;
    extern bool intVBL;
    extern bool intHBL;


    struct PPUControl{
        bool lcdEnabled;
        bool windowDisplay;
        bool objDisplay;
        bool windowBgDisplay;
        int objH;
        int objW;
    };

    struct AddrRange{
        int lower, higher;
    };


    extern PPUControl LCDC;

    enum PPU_MODE {
        OAM = 2,
        PIXTX = 3,
        HBLANK = 0,
        VBLANK = 1
    };
    extern PPU_MODE currentPpuMode;

    struct OAMEntry{
        int posX, posY, tileNo;
        bool priority;
        bool flipX, flipY;
    };



    void handlePPU(int);
    void updateSTAT();
    void updateVariables();

}