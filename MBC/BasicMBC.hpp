#pragma once
#include <cstdint>
#include <cstddef>
#include <vector>
#include <Headers/Structures.hpp>

class RAM;
class Debugger;

class BasicMBC {
protected:
    //  Emulator modules
    RAM* memory;
    Debugger* debugger;

    //  RAM bank/rom bank count
    size_t extRamBankCount = 0, romBankCount = 0;
    //  RAM/ROM physical size
    size_t flashSize = 0, romSize = 0;
    //  Feature flags
    MBCFlags flags = 0;

public:
    void bindMBC(RAM* newMem, Debugger* newDebug);

    size_t getFlashBankCount();

    void setFlashBankCount(size_t count);

    void setFlashSize(size_t size);

    size_t getFlashSize();

    void setROMBankCount(size_t count);

    size_t getROMBankCount();

    bool supports(MBCFlags flags);

    virtual bool handleWriteMBC(uint16_t address, uint8_t byte) { return false; };

    virtual uint8_t handleReadMBC(uint16_t address) { return 0xFF; };

    virtual bool flashEnabled() { return false; };

    virtual void mountBanks() {};

    virtual ~BasicMBC() = default;
};

