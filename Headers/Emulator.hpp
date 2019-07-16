#pragma once
#include "Headers/PPU.hpp"
#include "Headers/Debugger.hpp"
#include "Headers/Config.hpp"
#include "Headers/Display.hpp"
#include "Headers/LR35902.hpp"
#include "Headers/RAM.hpp"

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

    //  Configuration
    bool emuHalt = false, shouldReload = false, romChangeRequested = false;
    //  Requested ROM filename
    std::string newRomFile;

public:
    void init();

    void start();

    void reload();

    Config *getConfig();

    Debugger *getDebugger();

    PPU *getPPU();

    RAM *getMemory();

    Display *getDisplay();

    LR35902 *getCPU();

    void halt();

    bool isHalted();

    void triggerReload();

    void handleEventsSDL();

    void requestChangeROM(std::string newROM);

    static int SDLEventAddedCallback(void* usrData, SDL_Event* event);

};
