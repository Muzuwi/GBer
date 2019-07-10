#pragma once

#include <cstdint>
#include <SDL2/SDL.h>
#include <assert.h>
#include <chrono>
#include "Headers/GameboyDefinitions.hpp"


class Emulator;

class Display{
    //  Emulator object
    Emulator* emulator;

    //  SDL Window
    SDL_Window* gberWindow;
    SDL_Renderer* gberRenderer;
    int gberWindowID;

    //  Contains the state (pressed/unpressed) of joypad keys
    Keypad joypadState;

    //  Scale setting
    int scale = 4;

    //  Has the initialization of SDL failed?
    bool graphicsInitializationFailure;

    //  Color palette
    SDL_Color gameboyColors[4];

    uint32_t framebuffer[GAMEBOY_SCREEN_WIDTH*GAMEBOY_SCREEN_HEIGHT] = {0};

    //  Timepoint at which the last frame was drawn
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> lastFrameTimepoint;
    //  Frame duration
    std::chrono::milliseconds frameDuration;
public:
    void bind(Emulator* newEmulator);
    bool initializeGraphicLibs();
    void updateWindow();
    void appendBufferedLine(uint32_t* lineData, size_t size, size_t line);
    void createGameWindow();
    void destroyGameWindow();
    void clearWindow();
    void reload();
    SDL_Color getColor(size_t n);
    Keypad* getJoypad();
    int64_t getFrameDuration();
    void setFrameTime();
    void handleEvent(SDL_Event* event);
    unsigned int getWindowID();
    SDL_Window* getWindow();
};