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

enum GBerKeyBinding{
    KeyUp = 0,
    KeyDown = 1,
    KeyLeft = 2,
    KeyRight = 3,
    KeyA = 4,
    KeyB = 5,
    KeySel = 6,
    KeyStart = 7
};

//  Type used for mbc features
typedef unsigned int MBCFlags;

enum MBCFlags_{
    MBC_MBC1   = 1 << 1,
    MBC_MBC2   = 1 << 2,
    MBC_MBC3   = 1 << 3,
    MBC_MBC5   = 1 << 4,
    MBC_Timer  = 1 << 5,
    MBC_Flash  = 1 << 6,
    MBC_Rumble = 1 << 7,
    MBC_Battery = 1 << 8
};