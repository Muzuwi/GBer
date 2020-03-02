#pragma once
#include <vector>
#include <cstdint>
#include <string>
#include <map>
#include <memory>
#include "Headers/GameboyDefinitions.hpp"
#include "Headers/Utils.hpp"
#include "MBC/BasicMBC.hpp"

class Emulator;

class RAM{
    //  Pointer to the emulator object this memory object is bound to
    Emulator* emulator;

    //  RAM storage
    std::vector<uint8_t> memory { };
    std::vector<uint8_t> flash { };

    //  ROM file
    std::vector<uint8_t> romFile { };

    //  Currently used MBC object
    std::unique_ptr<BasicMBC> mbc;

    //  MBC Type that is currently in use
    MBC_Type mbcType;

    //  It's 2019 and there's still no way to get a string from an enum
    std::map<MBC_Type, std::string> controllerTypeLabel = {
            {ROM , "ROM"},
            {MBC1 , "MBC1"},
            {MBC1_RAM , "MBC1_RAM"},
            {MBC1_RAM_BAT , "MBC1_RAM_BAT"},
            {MBC2 , "MBC2"},
            {MBC2_BAT , "MBC2_BAT"},
            {ROM_RAM , "ROM_RAM"},
            {ROM_RAM_BAT , "ROM_RAM_BAT"},
            {MMM01 , "MMM01"},
            {MMM01_RAM , "MMM01_RAM"},
            {MMM01_RAM_BAT , "MMM01_RAM_BAT"},
            {MBC3_TIM_BAT , "MBC3_TIM_BAT"},
            {MBC3_RAM_TIM_BAT , "MBC3_RAM_TIM_BAT"},
            {MBC3 , "MBC3"},
            {MBC3_RAM , "MBC3_RAM"},
            {MBC3_RAM_BAT , "MBC3_RAM_BAT"},
            {MBC5 , "MBC5"},
            {MBC5_RAM , "MBC5_RAM"},
            {MBC5_RAM_BAT , "MBC5_RAM_BAT"},
            {MBC5_RUMBLE , "MBC5_RUMBLE"},
            {MBC5_RAM_RUMBLE , "MBC5_RAM_RUMBLE"},
            {MBC5_RAM_BAT_RUMBLE , "MBC5_RAM_BAT_RUMBLE"},
            {MBC6_RAM_BAT , "MBC6_RAM_BAT"},
            {MBC7_RAM_BAT_ACCEL , "MBC7_RAM_BAT_ACCEL"},
            {POCKET_CAM , "POCKET_CAM"},
            {TAMA5 , "TAMA5"},
            {HUC3 , "HUC3"},
            {HUC1_RAM_BAT , "HUC1_RAM_BAT"}
    };

    struct{
        uint16_t source;
        unsigned int nextByte, clockCounter;
        bool completed = true;
    } CurrentOAMTransfer;

    void beginTransferOAM(uint16_t source);


public:
    void bind(Emulator* newEmulator);
    uint8_t read(uint16_t address);
    uint8_t peek(uint16_t address);
    void poke(uint16_t address, uint8_t value);
    bool write(uint16_t address, uint8_t byte);
    void write16(uint16_t address, uint16_t value);
    bool insert(std::vector<uint8_t> src, uint16_t dest, size_t length, size_t offset, bool override=false);
    bool insert(const uint8_t* src, uint16_t dest, size_t length, size_t offset, size_t dataLength, bool override=false);
    void decodeHeader();
    void clear();
    void reload();
    void mountBanksRAM();
    void unmountRAM();
    void loadROM(std::string path);

    void updateTransfers(size_t cycles);

    uint8_t* getBaseMemoryPointer();
    uint8_t* getFlashBasePointer();
    uint8_t* getROMBasePointer();
    size_t getSizeROM();
    size_t getSizeFlash();
};