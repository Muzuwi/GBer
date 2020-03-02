#pragma once
#include <cstdint>
#include "MBC/BasicMBC.hpp"
#include "Headers/GameboyDefinitions.hpp"

class MBC1 : public virtual BasicMBC{

    //  Is the flash enabled?
    bool extRAMEnabled;
    //  Current MBC banking mode
    BankingMode currentBankingMode;
    //  Bank ID
    uint8_t bankNumberUpper2, bankNumberLower5;

    uint8_t getMountedBankROM() const;
    uint8_t getMountedBankRAM() const;

public:
    MBC_CONSTRUCTOR(MBC1) { };

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override { return extRAMEnabled; };

    void mountBanks() override;
};

