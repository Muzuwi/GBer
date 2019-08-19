#include <SDL2/SDL.h>
#include <iostream>
#include "Headers/APU.hpp"
#include "Headers/Emulator.hpp"

void APU::bind(Emulator *newEmu) {
    emulator = newEmu;
}

void APU::update() {

}

void APU::init() {
    SDL_AudioSpec requested, received;
    SDL_zero(requested);
    requested.freq = 48000;
    requested.format = AUDIO_F32;
    requested.channels = 2;
    requested.samples = 4096;
    audioDevice = SDL_OpenAudioDevice(NULL, 0, &requested, &received, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
    if(!audioDevice){
        emulator->getDebugger()->emuLog("Failed retrieving audio device! SDL Error: ", LOGLEVEL::WARN);
        emulator->getDebugger()->emuLog(SDL_GetError());
    } else {
        std::string device {"samplerate " + std::to_string(received.freq) + " Hz"
                            ", channels " + std::to_string(received.channels) +
                            ", samples " + std::to_string(received.samples)};
        emulator->getDebugger()->emuLog("Obtained audio device: " + device);
    }
}

