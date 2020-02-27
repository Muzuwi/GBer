#include "MBC/MBC5.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Debugger.hpp"

bool MBC5::handleWriteMBC(uint16_t address, uint8_t byte) {
    //  Write changes applied to the currently mounted bank in storage
    if(address >= 0xA000 && address <= 0xBFFF) {
        //  If RAM/RTC writing is enabled
        if (extRAMEnabled) {
            //  If in RAM bank mode
            auto flash = memory->getFlashBasePointer();
            //  If no other banks are present
            if (extRamBankCount < 4) flash[(address - 0xA000)] = byte;
                //  Otherwise change at the proper bank location
            else flash[(address - 0xA000) + 0x2000 * mountedBankNumberRAM] = byte;

            return true;
        } else {
            return false;
        }
    }

    //  Enabling extRAM and rtc registers
    if(address >= 0x0000 && address <= 0x1FFF){
        if(byte == 0x0A){
            extRAMEnabled = true;
        } else if(byte == 0){
            extRAMEnabled = false;
        }
        return false;
    }

    //  Remounting rom bank
    if(address >= 0x2000 && address <= 0x2FFF){
        //  Bank to change to
        unsigned int bank = (mountedBankNumberRAM & 0x100) | (byte & 0xFF);

        bank %= this->romBankCount;

        //  Switch banks only if necessary
        if(mountedBankNumberROM != bank){
            mountedBankNumberROM = bank;
            //debugger->emuLog("MBC5/ Switching to bank " + std::to_string(mountedBankNumberROM));
            assert(romBankCount >= mountedBankNumberROM);
            memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000*mountedBankNumberROM, memory->getSizeROM());
        }
        return false;
    }

    //  High bit of ROM bank number
    if(address >= 0x3000 && address <= 0x3FFF){
        //  Clamp
        byte &= 1;
        //  New resulting bank
        unsigned int bank = (byte << 8) | (mountedBankNumberROM & 0xFF);
        if(mountedBankNumberROM != bank){
            mountedBankNumberROM = bank;
            //debugger->emuLog("MBC5/ Switching to bank " + std::to_string(mountedBankNumberROM));
            assert(romBankCount >= mountedBankNumberROM);
            memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000*mountedBankNumberROM, memory->getSizeROM());
        }
        return false;
    }

    //  RAM Bank
    if(address >= 0x4000 && address <= 0x5FFF){
        //  RAM Bank Number
        if(extRamBankCount > 0 && byte != mountedBankNumberRAM && (byte >= 0 && byte <= 0x0F)){
//            debugger->emuLog("MBC5/ Remounted flash to " + std::to_string(byte) + ", from " + std::to_string(mountedBankNumberRAM));
            mountedBankNumberRAM = byte;
            assert(romBankCount >= mountedBankNumberRAM);
            memory->insert(memory->getFlashBasePointer(), 0xA000, 0x2000, 0x2000*mountedBankNumberRAM, this->flashSize);
        }
        return false;
    }

    return true;
}

uint8_t MBC5::handleReadMBC(uint16_t address) {
    return 0xFF;
}

bool MBC5::flashEnabled() {
    return extRAMEnabled;
}

void MBC5::mountBanks() {
    //  Mount banks 0 and 1 into memory
    memory->insert(memory->getROMBasePointer(), 0x0, 0x4000, 0, memory->getSizeROM());
    memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000, memory->getSizeROM());
}

MBC5::MBC5(MBCFlags config) {
    this->flags = config;
}
