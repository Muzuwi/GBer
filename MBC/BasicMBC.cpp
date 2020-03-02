#include "MBC/BasicMBC.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Debugger.hpp"

BasicMBC::BasicMBC(MBCFlags fl, std::vector<uint8_t> &rom_vec, std::vector<uint8_t> &flash_vec, Debugger &debug_ref,
                   size_t romCount, size_t flashCount) : debugger(debug_ref), rom(rom_vec), flash(flash_vec)
{
    flags = fl;
    flashBankCount = flashCount;
    romBankCount = romCount;
    flashSize = flashBankCount * 0x2000;
    romSize = romBankCount * 0x4000;
}
