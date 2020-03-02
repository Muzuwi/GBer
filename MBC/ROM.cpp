#include "MBC/ROM.hpp"

/*
 *  Dummy MBC for ROM banking
 */

bool ROM::handleWriteMBC(uint16_t address, uint8_t byte) {
    return !(address >= 0x0150 && address <= 0x7FFF);
}

uint8_t ROM::handleReadMBC(uint16_t address) {
    return rom[address];
}