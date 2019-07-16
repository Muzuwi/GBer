#pragma once
#include <string>
#include <iostream>
#include "Headers/Utils.hpp"

class Config{
    bool debugMode = false, clearState = false, noLog = false, customBoot = false, noGraphics = false;
    std::string romFilename, saveFilename, bootromFilename;
public:
    bool parseArgs(char* argv[], int argc);
    void setDefaults();
    bool isDebug();
    bool noLogging();
    bool shouldClearState();
    bool graphicsDisabled();
    std::string getFilename();
    std::string getSavename();
    bool setNewFilename(std::string filename);
};