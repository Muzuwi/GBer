#pragma once
#include <cassert>
#include "MBC/BasicMBC.hpp"
#include "Headers/GameboyDefinitions.hpp"

class MBC3 : public virtual BasicMBC {
    unsigned int mountedBankNumberROM,  //  Current ROM Bank number
                 mountedBankNumberRAM;  //  Current RAM bank number
    //  Is flash enabled?
    bool extRAMEnabled = false;

    //  Current banking mode for the A000-BFFF area
    BankingMode currentBankingMode = RAM_BANK;


public:
    MBC3(MBCFlags config);

    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override;

    void mountBanks() override;
};
