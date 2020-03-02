#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include "Headers/Structures.hpp"

#define MBC_CONSTRUCTOR(className) className(MBCFlags fl, std::vector<uint8_t> &rom_vec, std::vector<uint8_t> &flash_vec, Debugger &debug_ref, \
size_t romCount, size_t flashCount) : BasicMBC(fl, rom_vec, flash_vec, debug_ref, romCount, flashCount)

class RAM;
class Debugger;

class BasicMBC {
protected:
    Debugger& debugger;

    //  RAM bank/rom bank count
    size_t flashBankCount = 0, romBankCount = 0;
    //  RAM/ROM physical size
    size_t flashSize = 0, romSize = 0;
    //  Feature flags
    MBCFlags flags = 0;

    //  ROM/RAM
    std::vector<uint8_t> &rom;
    std::vector<uint8_t> &flash;

public:
    BasicMBC(MBCFlags flags, std::vector<uint8_t> &rom_vec, std::vector<uint8_t> &flash_vec, Debugger& debug_ref,
             size_t romBankCount, size_t flashBankCount);

    size_t getFlashBankCount() const { return flashBankCount; };
    size_t getFlashSize() const { return flashSize; };
    size_t getROMSize() const { return romSize; };
    size_t getROMBankCount() const { return romBankCount; };

    bool supports(MBCFlags flag) const { return (this->flags & flag); };

    virtual bool handleWriteMBC(uint16_t address, uint8_t byte) = 0;

    virtual uint8_t handleReadMBC(uint16_t address) = 0;

    virtual bool flashEnabled() = 0;

    virtual void mountBanks() = 0;

    virtual ~BasicMBC() = default;
};

