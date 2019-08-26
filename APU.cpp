#include <SDL2/SDL.h>
#include <iostream>
#include "Headers/APU.hpp"
#include "Headers/Emulator.hpp"

void APU::bind(Emulator *newEmu) {
    emulator = newEmu;
}

/*
 *  Updates...stuff
 */
void APU::update() {
    updateVariables();
}

/*
 *  Opens a new audio device
 */
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
    this->deviceSpec = received;
}

/*
 *  Updates internal variables from RAM
 */
void APU::updateVariables() {
    auto ram = emulator->getMemory()->getBaseMemoryPointer();

    //  Update SoundMode1 register data
    uint8_t nr10 = ram[NR10],
            nr11 = ram[NR11],
            nr12 = ram[NR12],
            nr13 = ram[NR13],
            nr14 = ram[NR14];
    //  Sweep register
    soundMode1Register.sweepRegister.sweepTime = (SweepTime)((nr10 & 0b01110000 ) >> 4);
    soundMode1Register.sweepRegister.sweepMode = (SweepMode)((nr10 & 0b00001000) >> 3);
    soundMode1Register.sweepRegister.sweepCount = nr10 & 0b00000111;

    //  Duty
    soundMode1Register.dutyRegister.wavePatternDuty = (Duty)((nr11 & 0b11000000) >> 6);
    soundMode1Register.dutyRegister.soundLength = (Duty)(nr11 & 0b00111111);

    //  Envelope
    soundMode1Register.envelopeRegister.initialVolume = (nr12 & 0b11110000) >> 4;
    soundMode1Register.envelopeRegister.envelopeMode = (EnvelopeMode)((nr12 & 0b00001000) >> 3);
    soundMode1Register.envelopeRegister.envelopeSweepCount = nr12 & 0b00000111;

    //  Frequency
    soundMode1Register.frequency = ((nr14 & 0b00000111) << 8) | (nr13);
    soundMode1Register.initial = (bool)(nr14 & 0b10000000);
    soundMode1Register.repeat = (bool)(nr14 & 0b01000000);
}

SoundMode1Register *APU::getSM1() {
    return &this->soundMode1Register;
}



