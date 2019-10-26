#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <variant>
#include <SDL2/SDL.h>
#include "Headers/Utils.hpp"
#include "Headers/Structures.hpp"

class Config{
    bool debugMode = false, clearState = false, noLog = false, customBoot = false, noGraphics = false;
    std::string romFilename, saveFilename, bootromFilename;
    std::vector<std::variant<int, bool, std::string>> config;

    SDL_Keycode keybindings[8] {
        SDLK_UP,
        SDLK_DOWN,
        SDLK_LEFT,
        SDLK_RIGHT,
        SDLK_z,
        SDLK_x,
        SDLK_a,
        SDLK_s
    };


public:
    bool parseArgs(char* argv[], int argc);
    void setDefaults();
    bool isDebug();
    void toggleDebug();
    bool noLogging();
    bool shouldClearState();
    bool graphicsDisabled();
    std::string getFilename();
    std::string getSavename();
    bool setNewFilename(std::string filename);
    SDL_Keycode getKeyBinding(GBerKeyBinding key);
    void setKeyBinding(GBerKeyBinding bind, SDL_Keycode in_key);
};