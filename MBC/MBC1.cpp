#include "MBC/MBC1.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Debugger.hpp"

bool MBC1::flashEnabled(){
    return extRAMEnabled;
}

uint8_t MBC1::handleReadMBC(uint16_t address) {
    return 0xFF;
}

bool MBC1::handleWriteMBC(uint16_t address, uint8_t byte) {
    auto flash = memory->getFlashBasePointer();

    //  RAM Bank writing
    if (address >= 0xA000 && address <= 0xBFFF) {

//        std::string text = "MBC1/ Writing to extRAM while it is ";
//        if(extRAMEnabled){
//            text += "enabled";
//        } else {
//            text += "disabled";
//        }
//        debugger->emuLog(text);

        if (extRAMEnabled) {
            //  Mode 0 makes the extRAM area bank 0
            if (currentBankingMode == ROM_BANK) {
                flash[(address - 0xA000)] = byte;
                return true;
            } else {
                if (extRamBankCount < 4) {
                    flash[(address - 0xA000)] = byte;
                } else {
                    flash[(address - 0xA000) + 8192 * bankNumberUpper2] = byte;
                }
                return true;
            }
        } else {
            return false;
        }
    }

    //  RAM Enable
    if (address >= 0x0 && address <= 0x1FFF) {

//        std::string text;
//        if((byte & 0xF) == 0x0A){
//            text = "MBC1/ Turning on flash";
//        } else {
//            text = "MBC1/ Turning off flash";
//        }
//        debugger->emuLog(text);

        extRAMEnabled = ((byte & 0xF) == 0x0A);

        //  Remount the bank if turning extRAM on caused the bank number to change
        if ((byte & 0xF) == 0x0A) {
            bankNumberUpper2 = 0;
            if (mountedBankNumber != bankNumberLower5) {
                mountedBankNumber = bankNumberLower5;
//                debugger->emuLog("Remounting to bank " + std::to_string(mountedBankNumber) + " due to banking mode change");
                assert(memory->getSizeROM() / 0x4000 >= mountedBankNumber);
                memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000 * mountedBankNumber, memory->getSizeROM(), false);
            }
        }
        return false;
    }

    //  ROM Bank Number
    if (address >= 0x2000 && address <= 0x3FFF) {
        //  Only the lower 5 bits are used
        //  Aditionally, values larger than the amount of banks in the rom
        //  overflow.
        uint8_t prev = mountedBankNumber;
        byte = (byte % romBankCount) & 0b11111;

//        std::string text = "MBC1/ hookWriteMBC(" + Utils::decHex(address) + ", " + Utils::decHex(byte,2) + ")";
//        debugger->emuLog(text);

        uint8_t bankNumber = 0;
        if (byte == 0) bankNumber = 1;
        else if (byte == 0x20 || byte == 0x40 || byte == 0x60) bankNumber = byte + 1;
        else bankNumber = byte;
        bankNumberLower5 = bankNumber;

        mountedBankNumber = ((bankNumberUpper2 << 5) | (bankNumberLower5)) % romBankCount;

        //  Switch banks only if necessary
        if (prev != mountedBankNumber) {
//            debugger->emuLog("MBC1/ Switching to bank " + std::to_string(mountedBankNumber));
            assert(romBankCount >= mountedBankNumber);
            memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000 * mountedBankNumber, memory->getSizeROM(), false);
        }
        return false;
    }

    //  RAM bank number/Upper ROM bank number depending on the current banking mode
    if (address >= 0x4000 && address <= 0x5FFF) {
        if (currentBankingMode == ROM_BANK) {

//            std::string text = "MBC1/ hookWriteMBC(" + Utils::decHex(address) + ", " + Utils::decHex(byte,2) + ")";
//            debugger->emuLog(text);

            uint8_t prev = mountedBankNumber;
            byte &= 0b11;
            bankNumberUpper2 = byte;

            mountedBankNumber = ((bankNumberUpper2 << 5) | (bankNumberLower5)) % romBankCount;

            //  Switch banks if necessary
            if (prev != mountedBankNumber) {
//                debugger->emuLog("MBC1/ Switching to bank " + std::to_string(mountedBankNumber));
                assert(memory->getSizeROM() / 0x4000 >= mountedBankNumber);
                memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000 * mountedBankNumber, memory->getSizeROM(), false);
            }
        } else if (currentBankingMode == RAM_BANK) {
            byte &= 0b11;
            bankNumberUpper2 = byte;
//            debugger->emuLog("Switching extRAM bank to " + std::to_string(bankNumberUpper2));

            //  TODO: Handle potential invalid save corruption/wrong sizes
            if (extRamBankCount <= 4) memory->insert(memory->getFlashBasePointer(), 0xA000, memory->getSizeFlash(), 0, memory->getSizeFlash());
            else {
                assert(bankNumberUpper2 * 8192 <= memory->getSizeFlash());
                memory->insert(memory->getFlashBasePointer(), 0xA000, 8192, bankNumberUpper2 * 8192, memory->getSizeFlash());
            }
        }
        return false;
    }

    return true;
}

/*
 *  Mounts the initial banks
 */
void MBC1::mountBanks() {
    //  Mount banks 0 and 1 into memory
    memory->insert(memory->getROMBasePointer(), 0x0, 0x4000, 0, memory->getSizeROM());
    memory->insert(memory->getROMBasePointer(), 0x4000, 0x4000, 0x4000, memory->getSizeROM());
}

MBC1::MBC1(MBCFlags config) {
    this->flags = config;
}
