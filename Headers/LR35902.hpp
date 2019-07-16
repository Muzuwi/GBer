#pragma once

#include <cstdint>
#include <string>
#include "Headers/GameboyDefinitions.hpp"
#include "Headers/Structures.hpp"

class Emulator;

class LR35902{
    //  Emulator object the CPU is bound to
    Emulator* emulator;

    //  Registers
    Reg Registers;

    bool step = true,
            continueExec = false,
            shouldEnableInts = false,
            cpuHalt = false,
            shouldDisableInts = false,
            stopped = false;
    //  CPU and timer frequencies
    unsigned int freq = CPU_FREQUENCY, freqDiv = TIMER_FREQUENCY;
    double timerFreq;

    //  CPU cycle count and timer cycle count
    int64_t cycles = 0, cyclesSinceStart = 0, prevCycles = 0;
    int64_t timerCycles = 0, divCycles = 0;
public:
    void bind(Emulator* newEmulator);
    void start();
    void reload();
    inline void decodeInstruction(uint8_t prefix, uint8_t op, uint8_t immediate);
    int cycle();
    void HandleInterrupt(std::string intName, unsigned char intVector, unsigned int mask);
    void clearState();
    void updateTimers(int64_t delta);
    inline uint16_t readAF();
    inline uint16_t readHL();
    inline uint16_t readBC();
    inline uint16_t  readDE();
    inline void writeHL(uint16_t data);
    inline void writeBC(uint16_t data);
    inline void writeDE(uint16_t data);
    inline void writeAF(uint16_t data);
    inline void copyToC(int mask);
    inline void DAA();
    inline void EI();
    inline void DI();

    void setTimerFrequency(double timer);
    double getTimerFrequency();
    unsigned int getFrequencyCPU();

    Reg getRegisters();
    bool doStep();
    void setStep(bool val);
    void setContinueExec(bool val);
    bool isCPUHalted();
    bool isStopped();
    void wakeFromStop();
};