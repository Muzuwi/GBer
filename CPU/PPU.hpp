#pragma once
namespace PPU {
    extern int LY, SCX, SCY;
    extern bool intLYC;
    extern bool intOAM;
    extern bool intVBL;
    extern bool intHBL;
    extern bool LYC_eq;

    struct AddrRange{
        int lower, higher;
    };

    enum PPU_MODE {
        OAM = 2,
        PIXTX = 3,
        HBLANK = 0,
        VBLANK = 1
    };

    extern PPU_MODE currentPpuMode;
    extern AddrRange windowTileMap, bgTileMap, bgWindowTileData;
    extern int64_t cyclesSinceModeSwitch;
    extern int vblankLines, pixelTransferFinishedFetches;
    //void handlePPU(int64_t);
    void updateVariables();
    void updateStat();
}