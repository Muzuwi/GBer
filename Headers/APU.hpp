#pragma once

class Emulator;

class APU{
    Emulator* emulator;
    //  Requested device ID
    SDL_AudioDeviceID audioDevice;

public:
    void bind(Emulator* newEmu);
    void init();
    void update();
};