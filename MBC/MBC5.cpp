#include "MBC/MBC5.hpp"
#include "Headers/Debugger.hpp"

uint8_t MBC5::handleReadMBC(uint16_t address) {
    if(address <= 0x3fff) {
        return rom[address];
    } else if (address >= 0x4000 && address <= 0x7fff) {
        return rom[getMountedBankROM()*0x4000 + (address-0x4000)];
    } else if(address >= 0xA000 && address <= 0xBFFF) {
        return flash[getMountedBankRAM()*0x2000 + (address - 0xA000)];
    }

    ASSERT_NOT_REACHED("Failed to process read MBC5");
}

bool MBC5::handleWriteMBC(uint16_t address, uint8_t byte) {
    //  Write changes applied to the currently mounted bank in storage
    if (address >= 0xA000 && address <= 0xBFFF) {
        if (extRAMEnabled) {
            if (flashBankCount == 1) {
                flash.at((address - 0xA000)) = byte;
            } else {
                flash.at((address - 0xA000) + 0x2000 * getMountedBankRAM()) = byte;
            }
        }
        return false;
    }

    //  Enabling flash
    if (address >= 0x0000 && address <= 0x1FFF){
        if (byte == 0x0A){
            extRAMEnabled = true;
        } else if(byte == 0){
            extRAMEnabled = false;
        }
        return false;
    }

    //  ROM bank change
    if (address >= 0x2000 && address <= 0x2FFF){
        unsigned int bank = (mountedBankNumberAUX & 0x100) | (byte & 0xFF);
        mountedBankNumberAUX = bank;

        return false;
    }

    //  High bit of ROM bank number
    if (address >= 0x3000 && address <= 0x3FFF){
        byte &= 1; //  FIXME?: Test if any written values other than 0 cause the bit to be set
        unsigned int bank = (byte << 8) | (mountedBankNumberROM & 0xFF);
        mountedBankNumberROM = bank;

        return false;
    }

    //  RAM Bank change
    if (address >= 0x4000 && address <= 0x5FFF){
        if (byte <= 0x0F)
            mountedBankNumberAUX = byte;

        return false;
    }

    ASSERT_NOT_REACHED("Failed to process write MBC5");
}

void MBC5::mountBanks() {
    extRAMEnabled = false;
    mountedBankNumberROM = 1;
    mountedBankNumberAUX = 0;
}

inline size_t MBC5::getMountedBankROM() const {
    return mountedBankNumberROM;
}

inline size_t MBC5::getMountedBankRAM() const {
    return mountedBankNumberAUX;
}
