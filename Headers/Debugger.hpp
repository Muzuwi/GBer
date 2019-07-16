#pragma once
#include <string>
#include <iostream>

#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"

#include "Debugger/modulesAll.hpp"

class Emulator;

class Debugger{
    //  Emulator object
    Emulator* emulator;

    //  Debugger window
    SDL_Window* debuggerWindow;
    SDL_GLContext debuggerGLContext;
    int debuggerWindowID;

    //  Pointer to the imgui context
    ImGuiContext* context;

    //  Memory editor widget
    MemoryEditor memEdit;

    //  Debugger modules
    DebugBreakpoint breakpointControl;
    DebugCPU cpuState;
    DebugFlow flowWindow;
    DebugInterrupt interruptsWindow;
    DebugIO ioViewer;
    DebugPPU ppuWindow;
    //DebugSound soundWindow;
    DebugStack stackWindow;
    DebugVRAM vramViewer;
    DebugPerformance performanceDebugger;
    DebugAPU apuDebugger;

    //  ImGui IO
    ImGuiIO* io;

    //  Is the debugger closed?
    bool resetDockLayout = false, isPaused = false;

    //  Status text
    std::string menuText;
    unsigned int timer = 0;


public:
    //  Log window error level
    enum LEVEL{
        ERR=10,
        WARN=5,
        INFO=1
    };

    void bind(Emulator* newEmulator);
    void emuLog(std::string message, LEVEL lvl);
    void emuLog(std::string message);

    void createDebugWindow();

    void destroyDebugWindow();

    inline ImGuiDockNodeFlags startDockspace();

    inline void endDockspace();

    inline void setDefaultLayout();

    inline void pauseDebugger();

    inline void unpauseDebugger();

    void updateDebugWindow();

    void updateDebugWindowContents();

    void createDebugTooltip(const std::string text, unsigned int time);

    void createWindowSDL();

    void handleEvent(SDL_Event* event);

    unsigned int getWindowID();

    bool isDebuggerPaused();

    void handleAddressBreakpoint(uint16_t address);
    void handleInstructionBreakpoint(uint8_t op);
    void handleMemoryBreakpoint(uint16_t address);
    void handleMemoryBreakpoint(uint16_t address, uint8_t byte);

};
