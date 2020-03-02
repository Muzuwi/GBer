#pragma once
#include <cstdint>
#include "MBC/BasicMBC.hpp"

/*
 *  MBC5 handler class
 */
class MBC5 : public virtual BasicMBC{
    //  Is the flash enabled?
    bool extRAMEnabled;
    //  Mounted ROM/RAM bank
    unsigned int mountedBankNumberROM, mountedBankNumberAUX;

    size_t getMountedBankROM() const;
    size_t getMountedBankRAM() const;

public:
    MBC_CONSTRUCTOR(MBC5) { }

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override { return extRAMEnabled; };

    void mountBanks() override;
};