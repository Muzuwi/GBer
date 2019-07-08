#include "MBC/ROM.hpp"
#include "Headers/RAM.hpp"

/*
 *  Dummy MBC for ROM banking
 */

bool ROM::handleWriteMBC(uint16_t address, uint8_t byte) {
    return !(address >= 0x0150 && address <= 0x7FFF);
}

uint8_t ROM::handleReadMBC(uint16_t address) {
    return memory->getBaseMemoryPointer()[address];
}

bool ROM::flashEnabled() {
    return false;
}

void ROM::mountBanks() {

}
