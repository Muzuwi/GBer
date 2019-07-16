#pragma once
#include "MBC/BasicMBC.hpp"

/*
 *  Dummy MBC for ROM banking
 */

class ROM : public virtual BasicMBC{
    bool handleWriteMBC(uint16_t address, uint8_t byte) override;

    uint8_t handleReadMBC(uint16_t address) override;

    bool flashEnabled() override;

    void mountBanks() override;
};