#pragma once
#include <cstdint>
#include <cassert>
#include "Headers/GameboyDefinitions.hpp"
#include "MBC/BasicMBC.hpp"

class MBC1 : public virtual BasicMBC{

    //  Is the flash enabled?
    bool extRAMEnabled = false;
    //  Current MBC banking mode
    BankingMode currentBankingMode = ROM_BANK;
    //  Bank ID
    uint8_t bankNumberUpper2 = 0, bankNumberLower5 = 0, mountedBankNumber = 0;

public:
    MBC1(MBCFlags config);

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override;

    void mountBanks() override;
};

