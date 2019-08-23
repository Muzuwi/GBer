#pragma once

#include <cstdint>
#include <cassert>
#include <chrono>
#include <SDL2/SDL.h>
#include <GL/gl3w.h>
#include "imgui/imgui.h"
#include "imgui/imgui_memory_editor.h"
#include "Headers/GameboyDefinitions.hpp"
#include "Debugger/DebuggerModules.hpp"

class Emulator;

class Display{
    //  Emulator object
    Emulator* emulator;

    //  SDL Window
    SDL_Window* gberWindow;
    int gberWindowID;
    SDL_GLContext gberGLContext;
    //  Overlay
    ImGuiContext* overlayContext;

    //  OpenGL variables
    GLuint gameboyFramebuffer, gameboyTexture;

    //  Contains the state (pressed/unpressed) of joypad keys
    Keypad joypadState;

    //  Scale setting
    int scale = 5, newScale = 5;

    //  Has the initialization of SDL failed?
    bool graphicsInitializationFailure;
    //  Show overlay?
    bool overlayOpen = false;

    //  Color palette
    SDL_Color gameboyColors[4];

    //  Gameboy screen framebuffer
    //  WARNING: The framebuffer is inverted, to compensate for OpenGL's texture coordinates
    uint32_t framebuffer[GAMEBOY_SCREEN_WIDTH*GAMEBOY_SCREEN_HEIGHT] = {0};

    //  Initialization and termination
    bool initializeSDL();
    void initializeImGui();
    void createGameWindow();
    void setGLState();
    void terminateSDL();
    void terminateImGuiContext();
    void destroyGameWindow();

    //  Update functions
    void drawMenuBar();
    void drawCommon();
    void createDefaultDockspace(ImGuiID existingDockspace);
    inline ImGuiDockNodeFlags startDebuggerDockspace();

    void drawDebuggerContents();
    void drawWindowDebuggerEnabled();

    void drawOverlayContents();
    void drawWindowGameOnly();

    void handleKeyEvents(SDL_Event* event);

    //  Helper flags for ImGui windows
    struct{
        bool showDemoWindow = false;
        bool showGraphicsConfig = false;
        bool reloadROM = false;
        bool resetLayout = false;
        bool requestWindowSizeChange = false;

    } WindowFlags;

    //  Debugger mode windows
    struct{
        //  Memory editor widget
        MemoryEditor memEdit;
        //  Debugger widgets
        DebugCPU cpuDebugger;
        DebugFlow flowControl;
        DebugInterrupts interruptControl;
        DebugIO ioViewer;
        DebugAPU apuViewer;
        DebugStack stack;
        DebugPPU ppuViewer;
        DebugVRAM vramViewer;
        DebugGameboyScreen gameboyScreen;
        DebugBreakpoints breakpointControl;
    } DebuggerWindows;

    //  Used for the menu bar tooltip
    struct{
        std::string text;
        unsigned int frameCount;
    } Tooltip;

public:
    void bind(Emulator* newEmulator);

    //  Main init/quit functions
    void initialize();
    void terminate();

    //  Update window contents
    void updateWindow();

    //  Modify window contents
    void appendBufferedLine(uint32_t* lineData, size_t size, size_t line);
    void clearWindow();
    void reload();

    //  Debugging
    void eventDebugStatusSwitched();
    void createDebugTooltip(std::string text, unsigned int frames);

    //  Frame limiting
    std::chrono::milliseconds updateFrameTime();

    //  Getters
    SDL_Color getColor(size_t n);
    Keypad* getJoypad();
    GLuint getScreenTexture();
    int getScale();

    //  Event handling helpers
    void handleEvent(SDL_Event* event);
};