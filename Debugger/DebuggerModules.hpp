#pragma once
#include <string>
#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include "Debugger/DebuggerWindow.hpp"

/*
 *  This file contains the declarations for all of the debugger widgets
 */

/*
 *  Register window
 */
class DebugCPU : public DebuggerWindow{
    bool checkZ = false, checkN = false, checkH = false, checkC = false;
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "CPU"; }
};


/*
 *  Flow control
 */
class DebugFlow : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "Flow Control"; }
};


/*
 *  Current interrupts window
 */
class DebugInterrupts : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "Interrupts"; }
};


/*
 *  I/O register viewer
 */
class DebugIO : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "I/O Viewer"; }
};


/*
 *  APU viewer
 */
class DebugAPU : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "APU"; }
};


/*
 *  Stack viewer
 */
class DebugStack : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "Stack"; }
};


/*
 *  PPU status viewer
 */
class DebugPPU : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
public:
    const char* getWindowName() override { return "PPU Status"; }
};


/*
 *  VRAM tile viewer
 */
class DebugVRAM : public DebuggerWindow{
    //  Should the tiles be flipped when displayed?
    bool flipTilesX = false, flipTilesY = false;

    //  Buffers for the textures
    unsigned int tileMapBuffer[16*8*3*64] {0};
    unsigned int backgroundMapBuffer[0x20*0x20*64] {0};

    //  GL texture IDs for tile map and background map display
    GLuint textureID, backgroundMapTextureID;

    //  Tile/Background map texture width and height
    unsigned int tileMapWidth = 16*8, tileMapHeight = 8*3*8,
            backgroundMapWidth = 0x20*8, backgroundMapHeight = 0x20*8;

    //  Colors to use when drawing
    SDL_Color colors[4];

    inline void updateMap(Emulator* emulator);
    inline void updateTileData(Emulator* emulator);
    void updateWindowContents(Emulator* emulator) override;
    ImGuiWindowFlags getWindowFlags() override { return (ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize); }
public:
    void init();
    const char* getWindowName() override { return "VRAM Viewer"; }
};


/*
 *  Breakpoint control
 */
class DebugBreakpoints : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
    ImGuiWindowFlags getWindowFlags() override { return (ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize); }
public:
    const char* getWindowName() override { return "Breakpoints"; }
};


/*
 *  Gameboy Screen
 */
class DebugGameboyScreen : public DebuggerWindow{
    void updateWindowContents(Emulator* emulator) override;
    ImGuiWindowFlags getWindowFlags() override { return (ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize); }
public:
    const char* getWindowName() override { return "Gameboy Screen"; }
};

