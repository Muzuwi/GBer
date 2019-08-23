#pragma once
#include <cassert>
#include "MBC/BasicMBC.hpp"

/*
 *  MBC5 handler class
 */
class MBC5 : public virtual BasicMBC{
    //  Is the flash enabled?
    bool extRAMEnabled = false;
    //  Mounted ROM/RAM bank
    unsigned int mountedBankNumberROM = 0, mountedBankNumberRAM = 0;
public:
    MBC5(MBCFlags config);

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override;

    void mountBanks() override;
};