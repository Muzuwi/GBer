#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <variant>
#include "Headers/Utils.hpp"

class Config{
    bool debugMode = false, clearState = false, noLog = false, customBoot = false, noGraphics = false;
    std::string romFilename, saveFilename, bootromFilename;
    std::vector<std::variant<int, bool, std::string>> config;

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
};