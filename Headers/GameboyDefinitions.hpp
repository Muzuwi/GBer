#pragma once

#define CPU_FREQUENCY 4194304
#define TIMER_FREQUENCY 16384


enum KeyBitMap{
    RIGHT = 0x1,
    A_BT = 0x1,

    LEFT = 0x2,
    B_BT = 0x2,

    UP = 0x4,
    SELECT = 0x4,

    DOWN = 0x8,
    START = 0x8,
};

struct OAM_Entry{
    uint8_t posX, posY;
    uint8_t patternNumber;
    bool priority, yFlip, xFlip, obj1Palette;
};

struct AddrRange{
    int lower, higher;
};

struct Keypad{
    bool lP, rP, uP, dP;
    bool startP, selP, aP, bP;
};


enum PPU_MODE {
    OAM = 2,
    PIXTX = 3,
    HBLANK = 0,
    VBLANK = 1
};

struct LCDController{
    bool lcdEnable, windowEnable, objEnable, bgWindowDisplayPriority, tileAccessModeSigned;
    AddrRange windowTileMap;
    AddrRange bgWindowTileData;
    AddrRange bgTileMap;
    uint8_t objHeight;
};

enum MBC_Type{
    ROM = 0x0,
    MBC1 = 0x01,
    MBC1_RAM = 0x02,
    MBC1_RAM_BAT = 0x03,
    MBC2 = 0x05,
    MBC2_BAT = 0x06,
    ROM_RAM = 0x08,
    ROM_RAM_BAT = 0x09,
    MMM01 = 0x0B,
    MMM01_RAM = 0x0C,
    MMM01_RAM_BAT = 0x0D,
    MBC3_TIM_BAT = 0x0F,
    MBC3_RAM_TIM_BAT = 0x10,
    MBC3 = 0x11,
    MBC3_RAM = 0x12,
    MBC3_RAM_BAT = 0x13,
    MBC5 = 0x19,
    MBC5_RAM = 0x1A,
    MBC5_RAM_BAT = 0x1B,
    MBC5_RUMBLE = 0x1C,
    MBC5_RAM_RUMBLE = 0x1D,
    MBC5_RAM_BAT_RUMBLE = 0x1E,
    MBC6_RAM_BAT = 0x20,
    MBC7_RAM_BAT_ACCEL = 0x22,
    POCKET_CAM = 0xFC,
    TAMA5 = 0xFD,
    HUC3 = 0xFE,
    HUC1_RAM_BAT = 0xFF,
};

enum BankingMode{
    ROM_BANK,
    RAM_BANK,
    RTC_BANK
};

/*
 *  APU register structures
 */

enum SweepTime{
    Sweep_Off = 0,
    Sweep_1_128th,
    Sweep_2_128th,
    Sweep_3_128th,
    Sweep_4_128th,
    Sweep_5_128th,
    Sweep_6_128th,
    Sweep_7_128th = 7,
};

enum SweepMode{
    Sweep_Addition = 0,
    Sweep_Subtraction = 1
};

enum Duty{
    Duty_12_5 = 0,
    Duty_25,
    Duty_50,
    Duty_75 = 3
};

enum EnvelopeMode{
    Envelope_Attenuate = 0,
    Envelope_Amplify = 1
};

struct SoundMode1SweepRegister{
    SweepTime sweepTime;
    SweepMode sweepMode;
    uint8_t sweepCount : 3;
};

struct SoundMode1DutyRegister{
    Duty wavePatternDuty;
    uint8_t soundLength : 6;
};

struct SoundMode1EnvelopeRegister{
    uint8_t initialVolume : 4;
    EnvelopeMode envelopeMode;
    uint8_t envelopeSweepCount : 3;
};

struct SoundMode1Register{
    SoundMode1SweepRegister sweepRegister;
    SoundMode1DutyRegister dutyRegister;
    SoundMode1EnvelopeRegister envelopeRegister;
    uint16_t frequency : 11;
    bool initial;
    bool repeat;
};



//  IO Registers

#define DIV 0xFF04
#define TIMA 0xFF05
#define TMA 0xFF06
#define TAC 0xFF07

#define LCDC 0xFF40
#define STAT 0xFF41
#define SCY 0xFF42
#define SCX 0xFF43
#define LY 0xFF44
#define LYC 0xFF45
#define DMA 0xFF46
#define WY 0xFF4A
#define WX 0xFF4B

#define IE 0xFFFF
#define IF 0xFF0F

#define IF_VBLANK 0x1
#define IF_LCDC 0x2
#define IF_TIMER 0x4
#define IF_SERIAL 0x8
#define IF_HILO 0x10

#define IE_VBLANK 0x1
#define IE_LCDC 0x2
#define IE_TIMER 0x4
#define IE_SERIAL 0x8
#define IE_HILO 0x10

#define SB 0xFF01
#define SC 0xFF02

#define NR10 0xFF10
#define NR11 0xFF11
#define NR12 0xFF12
#define NR13 0xFF13
#define NR14 0xFF14

#define NR21 0xFF16
#define NR22 0xFF17
#define NR23 0xFF18
#define NR24 0xFF19

#define NR30 0xFF1A
#define NR31 0xFF1B
#define NR32 0xFF1C
#define NR33 0xFF1D
#define NR34 0xFF1E

#define NR41 0xFF20
#define NR42 0xFF21
#define NR43 0xFF22
#define NR44 0xFF23

#define NR50 0xFF24
#define NR51 0xFF25
#define NR52 0xFF26


#define SCTRL 0xFF02
#define SDATA 0xFF01
#define P1 0xFF00

#define OAM_START 0xFE00
#define OAM_END 0xFE9F

#define BGP 0xFF47
#define OBP0 0xFF48
#define OBP1 0xFF49

#define SPRITE_TILE_START 0x8000

//  Behavior defines
#define GAMEBOY_SCREEN_WIDTH 160
#define GAMEBOY_SCREEN_HEIGHT 144

#define SPRITES_PER_LY 10

#define OAM_CYCLE_COUNT 63 * 4
#define HBLANK_CYCLE_COUNT 51 * 4
#define SCANLINE_CYCLE_COUNT (OAM_CYCLE_COUNT+HBLANK_CYCLE_COUNT)
#define VBLANK_CYCLE_COUNT (SCANLINE_CYCLE_COUNT)
#define VBLANK_TO_OAM_CYCLE_COUNT 8 * 4

#define OAM_TRANSFER_CYCLE_COUNT 160 * 4
#define OAM_TRANSFER_SIZE 0xA0
#define OAM_TRANSFER_TARGET 0xFE00
#define OAM_TRANSFER_CYCLES_PER_BYTE (OAM_TRANSFER_CYCLE_COUNT / OAM_TRANSFER_SIZE)