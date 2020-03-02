#include "MBC/MBC1.hpp"
#include "Headers/Debugger.hpp"

uint8_t MBC1::handleReadMBC(uint16_t address) {
    if (address <= 0x3fff) {
        return rom[address];
    } else if (address <= 0x7fff) {
        return rom[getMountedBankROM()*0x4000 + (address - 0x4000)];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (extRAMEnabled) {
            return flash[(address - 0xA000) + 0x2000 * getMountedBankRAM()];
        } else {
            return 0xFF;
        }
    }

    ASSERT_NOT_REACHED("Failed to process read MBC1");
}

bool MBC1::handleWriteMBC(uint16_t address, uint8_t byte) {
    //  RAM Bank writing
    if (address >= 0xA000 && address <= 0xBFFF) {
        if(extRAMEnabled) {
            if (flashBankCount == 1) {
                flash.at((address - 0xA000)) = byte;
            } else {
                flash.at((address - 0xA000) + 0x2000 * getMountedBankRAM()) = byte;
            }
        }

        return false;
    }

    //  RAM Enable
    if (address >= 0x0 && address <= 0x1FFF) {
        extRAMEnabled = ((byte & 0xF) == 0x0A);

        return false;
    }

    //  ROM Bank Number
    if (address >= 0x2000 && address <= 0x3FFF) {
        //  Only the lower 5 bits are used
        //  Aditionally, values larger than the amount of banks in the rom
        //  overflow.
        byte &= 0b11111;
        bankNumberLower5 = byte;

        return false;
    }

    //  RAM bank number/Upper ROM bank number depending on the current banking mode
    if (address >= 0x4000 && address <= 0x5FFF) {
        byte &= 0b11;
        bankNumberUpper2 = byte;

        return false;
    }

    //  Switching between banking modes
    if (address >= 0x6000 && address <= 0x7fff) {
        if(byte == 0x0) currentBankingMode = ROM_BANK;
        else if(byte == 0x01) currentBankingMode = RAM_BANK;

        return false;
    }


    ASSERT_NOT_REACHED("Failed to process read MBC3");
}

void MBC1::mountBanks() {
    bankNumberUpper2 = 0;
    bankNumberLower5 = 1;
    currentBankingMode = ROM_BANK;
    extRAMEnabled = false;
}

inline uint8_t MBC1::getMountedBankROM() const {
    uint8_t bankNumber;

    if (currentBankingMode == ROM_BANK) {
        bankNumber = ((bankNumberUpper2 << 5) | (bankNumberLower5));
    } else {
        bankNumber = bankNumberLower5;
    }

    if (bankNumber == 0) bankNumber = 1;
    else if (bankNumber == 0x20 || bankNumber == 0x40 || bankNumber == 0x60) bankNumber = bankNumber + 1;
    bankNumber %= romBankCount;

    return bankNumber;
}

inline uint8_t MBC1::getMountedBankRAM() const {
    if (currentBankingMode == ROM_BANK)
        return 0;
    else
        return (bankNumberUpper2);
}
