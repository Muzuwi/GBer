#pragma once
#include <cstdint>

//  Struct containing configuration for breakpoints
struct MemoryBreakpoint{
    MemoryBreakpoint(char16_t address, bool read, bool write, bool valBreakpoint=false, uint8_t value=0){
        addr = address;
        r = read;
        w = write;
        v = valBreakpoint;
        val = value;
    }
    char16_t addr;
    bool r = false, w = false, v = false;
    uint8_t val = 0;
};

struct Reg{
    uint16_t SP, PC;
    uint8_t A, F, B, C, D, E, H, L;
    bool IME;
};

//  Log window error level
enum LOGLEVEL{
    ERR=10,
    WARN=5,
    INFO=1
};
