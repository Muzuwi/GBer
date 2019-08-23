#include "MBC/BasicMBC.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Debugger.hpp"

//  Binds the MBC object to a memory object and a debug object
void BasicMBC::bindMBC(RAM* newMem, Debugger* newDebug){
    memory = newMem;
    debugger = newDebug;
}

void BasicMBC::setFlashBankCount(size_t count) {
    extRamBankCount = count;
}

void BasicMBC::setROMBankCount(size_t count) {
    romBankCount = count;
}

size_t BasicMBC::getFlashBankCount() {
    return extRamBankCount;
}

size_t BasicMBC::getROMBankCount() {
    return romBankCount;
}

void BasicMBC::setFlashSize(size_t size) {
    flashSize = size;
}

size_t BasicMBC::getFlashSize() {
    return flashSize;
}

bool BasicMBC::supports(MBCFlags flags) {
    return (this->flags & flags);
}
