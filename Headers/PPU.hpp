#pragma once
#include <cstdint>
#include <assert.h>
#include "Headers/GameboyDefinitions.hpp"

class Emulator;

class PPU{
    //  Pointer to emulator that is bound to this PPU object
    Emulator* emulator;

    //  Enabled interrupts
    bool intLYC, intOAM, intHBL, intVBL;
    //  PPU Cycle counter
    int64_t ppuCycles = 0;

    //  Contains LCD configuration
    LCDController lcdc;

    //  Current PPU mode
    PPU_MODE currentModePPU = OAM;

    inline uint8_t getSpritePalleteData(uint8_t num, bool pallete1);
    inline uint8_t getTilePalleteData(uint8_t num);
    inline void fetchCurrentLineBackground(uint8_t *pixels, uint8_t *raw);
    inline void fetchCurrentLineWindow(uint8_t *pixels, uint8_t *raw);
    inline void fetchCurrentLineSprite(uint8_t *pixels, const uint8_t *raw);
    inline void getRawTileData(uint8_t tileNumber,
                               unsigned int line,
                               uint8_t* array,
                               size_t arraySize,
                               unsigned int pixelOffset=0,
                               unsigned int bound=8,
                               bool xflip=false,
                               bool yflip=false,
                               uint16_t overrideDataAddress=0x0);
    inline void drawCurrentLine();

public:
    void bind(Emulator* newEmulator);
    void setModePPU(PPU_MODE mode);
    void clearCycles();
    void updateVariables();
    bool updateModePPU(int64_t delta);
    void reload();
    PPU_MODE getPPUMode();
    LCDController* getLCDC();
    int64_t getPPUCycles();
    void debugGetRawTileDataWrapper(uint8_t tileNumber,
                                         unsigned int line,
                                         uint8_t* array,
                                         size_t arraySize,
                                         unsigned int pixelOffset=0,
                                         unsigned int bound=8,
                                         bool xflip=false,
                                         bool yflip=false,
                                         uint16_t overrideDataAddress=0x0);
};