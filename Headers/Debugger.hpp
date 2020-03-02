#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <GL/gl3w.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_memory_editor.h"

#include "Headers/Structures.hpp"

#define ASSERT_NOT_REACHED(message) assert(false && message)


class Emulator;

class Debugger{
    //  Allow access to breakpoints from debugger breakpoint window
    friend class DebugBreakpoints;

    //  Emulator object
    Emulator* emulator;

    //  Breakpoint store
    std::vector<uint16_t> addressBreakpoints;
    std::vector<InstructionBreakpoint> instructionBreakpoints;
    std::vector<MemoryBreakpoint> memoryBreakpoints;

    bool checkAddressBreakpoint(uint16_t address);
    bool checkInstructionBreakpoint(InstructionBreakpoint op);
    bool checkMemoryBreakpoint(MemoryBreakpoint operation);

public:
    void bind(Emulator* newEmulator);
    void emuLog(std::string message, LOGLEVEL lvl);
    void emuLog(std::string message);

    void handleAddressBreakpoint(uint16_t address);
    void handleInstructionBreakpoint(InstructionBreakpoint op);
    void handleMemoryBreakpoint(MemoryBreakpoint operation);
    void addAddressBreakpoint(uint16_t address);
    void addOpBreakpoint(uint8_t op, bool isCB);
    void addMemoryBreakpoint(MemoryBreakpoint breakpoint);
};
