#include "MBC/BasicMBC.hpp"
#include "MBC/MBC3.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Debugger.hpp"

bool MBC3::handleWriteMBC(uint16_t address, uint8_t byte) {

    //  Write changes applied to the currently mounted bank in storage
    if(address >= 0xA000 && address <= 0xBFFF){
        //  If RAM/RTC writing is enabled
        if(extRAMEnabled){
            //  If in RAM bank mode
            if(currentBankingMode == RAM_BANK){
                auto flash = memory->getFlashBasePointer();
                //  If no other banks are present
                if(extRamBankCount < 4) flash[(address - 0xA000)] = byte;
                //  Otherwise change at the proper bank location
                else flash[(address - 0xA000) + 0x2000*mountedBankNumberRAM] = byte;
                return true;
            } else {
                //  TODO:
                return true;
            }
        } else{
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
    if(address >= 0x2000 && address <= 0x3FFF){
        //  Full 7 bytes used
        //  Aditionally, values larger than the amount of banks in the rom
        //  overflow.
        uint8_t prev = mountedBankNumberROM;
        byte = (byte % this->romBankCount) & 0b01111111;
        //std::cout << "MBC3/ hookWriteMBC(" << Math::decHex(address) << ", " << Math::decHex(byte,2) << ")\n";

        uint8_t bankNumber = 0;
        if(byte == 0) bankNumber = 1;
        else bankNumber = byte;

        mountedBankNumberROM = (bankNumber) % this->romBankCount;
        //  Switch banks only if necessary
        if(prev != mountedBankNumberROM){
            //debugger->emuLog("MBC3/ Switching to bank " + std::to_string(mountedBankNumberROM));
            assert(romBankCount >= mountedBankNumberROM);
            memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000*mountedBankNumberROM, memory->getSizeROM());
        }
        return false;
    }

    //  RAM Bank
    if(address >= 0x4000 && address <= 0x5FFF){
        if(byte >= 0 && byte <= 0x3 && extRAMEnabled){
            //  RAM Bank Number
            if(byte != mountedBankNumberRAM){
                currentBankingMode = RAM_BANK;
                debugger->emuLog("MBC3/ Remounted flash to " + std::to_string(byte) + ", from " + std::to_string(mountedBankNumberRAM));
                mountedBankNumberRAM = byte;
                assert(romBankCount >= mountedBankNumberRAM);
                memory->insert(memory->getFlashBasePointer(), 0xA000, 0x2000, 0x2000*mountedBankNumberRAM, this->flashSize);
            }
        } else if(byte >= 0x8 && byte <= 0xC && extRAMEnabled){
            //  RTC Registers
            debugger->emuLog("Writing RTC");
            currentBankingMode = RTC_BANK;
        }
        return false;
    }

    return true;
}

uint8_t MBC3::handleReadMBC(uint16_t address) {
    return 0xFF;
}

bool MBC3::flashEnabled() {
    return extRAMEnabled;
}

void MBC3::mountBanks() {
    debugger->emuLog("Mounting ROM banks 0 and 1...");
    //  Mount stuff
    //  Mount banks 0 and 1 into memory
    memory->insert(memory->getROMBasePointer(), 0x0, 0x4000, 0, memory->getSizeROM());
    memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000, memory->getSizeROM());
    debugger->emuLog("Mounting ROM banks 0 and 1 completed");

}
