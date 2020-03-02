#pragma once
#include <cstdint>
#include "MBC/BasicMBC.hpp"
#include "Headers/GameboyDefinitions.hpp"

class MBC3 : public virtual BasicMBC {
    unsigned int mountedBankNumberROM,  //  Current ROM Bank number
                 mountedBankNumberAUX;  //  Current RAM bank number
    //  Is flash enabled?
    bool extRAMEnabled;

    //  Current banking mode for the A000-BFFF area
    BankingMode currentBankingMode;

    size_t getMountedBankROM() const;
    size_t getMountedBankRAM() const;

public:
    MBC_CONSTRUCTOR(MBC3) { }

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override { return extRAMEnabled; };

    void mountBanks() override;
};
