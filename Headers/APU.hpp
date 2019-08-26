#pragma once
#include "Headers/GameboyDefinitions.hpp"

class Emulator;

class APU{
    Emulator* emulator;
    //  Requested device ID
    SDL_AudioDeviceID audioDevice;
    //  Audio device specs
    SDL_AudioSpec deviceSpec;

    //  Sound Mode registers
    SoundMode1Register soundMode1Register;


    void updateVariables();
public:
    void bind(Emulator* newEmu);
    void init();
    void update();
    SoundMode1Register* getSM1();
};