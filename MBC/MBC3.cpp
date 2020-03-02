#include "MBC/MBC3.hpp"
#include "Headers/Debugger.hpp"

uint8_t MBC3::handleReadMBC(uint16_t address) {
    if (address <= 0x3fff) {
        return rom[address];
    } else if (address <= 0x7fff) {
        return rom[getMountedBankROM()*0x4000 + (address - 0x4000)];
    } else if (address >= 0xA000 && address <= 0xBFFF) {
        if (!extRAMEnabled)
            return 0xFF;

        if (currentBankingMode == RAM_BANK) {
            return flash[(address - 0xA000) + 0x2000 * getMountedBankRAM()];
        } else if (currentBankingMode == RTC_BANK) {
            //  TODO:
            return 0xFF;
        }
    }

    ASSERT_NOT_REACHED("Invalid address passed to read MBC");
}

bool MBC3::handleWriteMBC(uint16_t address, uint8_t byte) {
    //  Write changes applied to the currently mounted bank in storage
    if (address >= 0xA000 && address <= 0xBFFF){
        if (currentBankingMode == RAM_BANK) {
            if (flashBankCount == 1) {
                flash.at((address - 0xA000)) = byte;
            } else {
                flash.at((address - 0xA000) + 0x2000 * getMountedBankRAM()) = byte;
            }
        } else {
            //  TODO: RTC Write
            ASSERT_NOT_REACHED("RTC unimplemented");
        }

        return false;
    }

    //  Enabling flash and rtc registers
    if (address >= 0x0000 && address <= 0x1FFF){
        if(byte == 0x0A){
            extRAMEnabled = true;
        } else if(byte == 0){
            extRAMEnabled = false;
        }
        return false;
    }

    //  Remounting rom bank
    if (address >= 0x2000 && address <= 0x3FFF){
        mountedBankNumberROM = byte & 0b01111111;
        return false;
    }

    //   RAM Bank
    if (address >= 0x4000 && address <= 0x5FFF){
        if (byte <= 0x3)
            currentBankingMode = RAM_BANK;
        else if (byte >= 0x8 && byte <= 0xC)
            currentBankingMode = RTC_BANK;

        mountedBankNumberAUX = byte;
        return false;
    }

    //  Latch clock data
    if (address >= 0x6000 && address <= 0x7fff) {
        //  TODO:
        return false;
    }

    ASSERT_NOT_REACHED("Failed to process write MBC3");
}

void MBC3::mountBanks() {
    mountedBankNumberROM = 1;
    mountedBankNumberAUX = 0;
    extRAMEnabled = false;
    currentBankingMode = RAM_BANK;
}

inline size_t MBC3::getMountedBankROM() const {
    uint8_t bankNumber = mountedBankNumberROM;
    if(bankNumber == 0) bankNumber = 1;
    bankNumber %= romBankCount;
    return bankNumber;
}

inline size_t MBC3::getMountedBankRAM() const {
    if (currentBankingMode == RAM_BANK)
        return mountedBankNumberAUX;
    else
        return 0; //  FIXME: ???
}
