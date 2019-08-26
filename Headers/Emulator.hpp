#pragma once
#include <string>
#include "Headers/PPU.hpp"
#include "Headers/Debugger.hpp"
#include "Headers/Config.hpp"
#include "Headers/Display.hpp"
#include "Headers/LR35902.hpp"
#include "Headers/RAM.hpp"
#include "Headers/APU.hpp"

class Emulator {
    //  Emulator memory class
    RAM memory;
    //  PPU class
    PPU ppu;
    //  Debugger class
    Debugger debug;
    //  Configuration class
    Config config;
    //  Display class
    Display display;
    //  CPU class
    LR35902 cpu;
    //  APU class
    APU apu;

    //  Configuration
    bool emuHalt = false, shouldReload = false, romChangeRequested = false;
    //  Requested ROM filename
    std::string newRomFile;

    void reload();

    bool handleChangeROM();

public:
    void init();

    void start();

    Config *getConfig();

    Debugger *getDebugger();

    PPU *getPPU();

    RAM *getMemory();

    Display *getDisplay();

    LR35902 *getCPU();

    APU *getAPU();

    void halt();

    void triggerBreak(std::string message);

    void triggerReload();

    void triggerToggleDebugger();

    void requestChangeROM(std::string newROM);
};
