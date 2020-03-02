//#include "Headers/Debugger_Rewrite.hpp"
#include <Headers/RAM.hpp>
#include "Headers/Emulator.hpp"
#include "Headers/RAM.hpp"
#include "MBC/MBC1.hpp"
#include "MBC/MBC3.hpp"
#include "MBC/MBC5.hpp"
#include "MBC/ROM.hpp"

//  Bind the module to an emulator object
void RAM::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

/*
 *  Read a byte from memory
 */
uint8_t RAM::read(uint16_t address){
    //  Breakpoints
    if (emulator->getConfig()->isDebug()){
        emulator->getDebugger()->handleMemoryBreakpoint(MemoryBreakpoint(address, true, false, false));
    }

    //  If an OAM transfer is in progress
    if(!CurrentOAMTransfer.completed){
        if(address < 0xFF80 || address > 0xFFFE){
            emulator->getDebugger()->emuLog(
                    "Invalid memory access, tried reading from " + Utils::decHex(address) + ", which is not HRAM, during an OAM transfer",
                    LOGLEVEL::WARN);
            return false;
        }
    }


    //  Joypad Reg
    if (address == P1){
        //  Get JoypadState
        auto JoypadState = emulator->getDisplay()->getJoypad();

        //  Directional keys
        if(!(memory[P1] & 0x10)){

            uint8_t byte = 0xF;
            if(JoypadState->lP) byte &= ~KeyBitMap::LEFT;
            if(JoypadState->rP) byte &= ~KeyBitMap::RIGHT;
            if(JoypadState->uP) byte &= ~KeyBitMap::UP;
            if(JoypadState->dP) byte &= ~KeyBitMap::DOWN;

            return byte;
        } else if(!(memory[P1] & 0x20)){

            uint8_t byte = 0xF;
            if(JoypadState->aP) byte &= ~KeyBitMap::A_BT;
            if(JoypadState->bP) byte &= ~KeyBitMap::B_BT;
            if(JoypadState->selP) byte &= ~KeyBitMap::SELECT;
            if(JoypadState->startP) byte &= ~KeyBitMap::START;

            return byte;

        } else {
            return 0;
        }
    }

    //  IE reg
    if (address == IE) {
        return memory[address];
    }

    //  ROM/RAM accesses go through the MBC
    if (address <= 0x7fff || (address >= 0xA000 && address <= 0xBFFF) ) {
        uint8_t byte = mbc->handleReadMBC(address);
//        emulator->getDebugger()->emuLog("MBC says: " + Utils::decHex(byte,2));
        return byte;
    }

    //  OAM inaccessible outside of blanking periods
    if (address >= 0xFE00 && address < 0xFEA0){
        if(!(emulator->getPPU()->getPPUMode() == PPU_MODE::VBLANK || emulator->getPPU()->getPPUMode() == PPU_MODE::HBLANK)){
            return 0xFF;
        } else {
            return memory[address];
        }
    }

    //  VRAM
    if (address >= 0x8000 && address <= 0x9fff) {
        return memory[address];
    }

    //  WRAM
    if (address >= 0xC000 && address <= 0xDFFF) {
        return memory[address];
    }

    //  Echo RAM
    if (address >= 0xE000 && address <= 0xFDFF) {
        return memory[address];
    }

    //  IO Registers
    if (address >= 0xFF00 && address <= 0xFFFE) {
        return memory[address];
    }

    // Most likely unspecified
    if ((address >= 0xFF4C && address < 0xFF80) || (address >= 0xFEA0 && address < 0xFF00) ){
        return 0xFF;
    }

    emulator->getDebugger()->emuLog("Read failed to redirect! " + Utils::decHex(address));
    assert(false);
}

/*
 *  Raw returns a byte from memory, without affecting timing and banking
 */
uint8_t RAM::peek(uint16_t address){
    return memory[address];
}

/*
 *  Raw write a byte to memory
 */
void RAM::poke(uint16_t address, uint8_t value){
    memory[address] = value;
}

/*
 *  Write a byte into memory
 */
bool RAM::write(uint16_t address, uint8_t byte){
    //  Breakpoints
    if(emulator->getConfig()->isDebug()){
        emulator->getDebugger()->handleMemoryBreakpoint(MemoryBreakpoint(address, false, true, true, byte));
    }

    //  If an OAM transfer is in progress
    if(!CurrentOAMTransfer.completed){
        if(address < 0xFF80 || address > 0xFFFE){
            emulator->getDebugger()->emuLog(
                    "Invalid memory access, tried writing to " + Utils::decHex(address) + ", which is not HRAM, during an OAM transfer",
                    LOGLEVEL::WARN);
//            memory[address] = byte;
            return false;
        }
    }

    //  Writing to MBC registers
    if (address <= 0x7FFF) {
        return mbc->handleWriteMBC(address, byte);
    }

    //  Writing to flash
    if (address >= 0xA000 && address <= 0xBFFF) {
        return mbc->handleWriteMBC(address, byte);
    }

    //  VRAM
    if (address >= 0x8000 && address <= 0x9FFF) {
        memory[address] = byte;
        return false;
    }

    //  OAM
    if (address >= 0xFE00 && address <= 0xFE9F) {
        memory[address] = byte;
        return false;
    }

    //  Undefined regions
    if ((address >= 0xFF4C && address < 0xFF80) ||
        (address >= 0xFEA0 && address < 0xFF00)) {
        return false;
    }

    //  Writing to Echo RAM
    if ((address >= 0xC000) && (address <= 0xDDFF)) {
        memory[address] = byte;
        memory[address + 0x2000] = byte;
        return false;
    }
    else if ((address >= 0xE000) && (address <= 0xFDFF)) {
        memory[address] = byte;
        memory[address - 0x2000] = byte;
        return false;
    }

    //  Writing to the rest of WRAM (outside of Echo RAM)
    if (address >= 0xDE00 && address <= 0xDFFF) {
        memory[address] = byte;
        return false;
    }


    //  Registers, TODO: Aggregate all the IO regs somewhere else
    if (address == DIV || address == LY) {
        return false;
    }
    if (address == DMA) {
        this->beginTransferOAM(byte << 8);
        return false;
    }
    if (address == SCTRL && byte == 0x81) {
        std::cout << read(SDATA);
        return false;
    }
    if (address == TAC) {
        switch (byte & 3) {
            case 0:
                emulator->getCPU()->setTimerFrequency(emulator->getCPU()->getFrequencyCPU() / 1024.0);
                break;
            case 1:
                emulator->getCPU()->setTimerFrequency(emulator->getCPU()->getFrequencyCPU() / 16.0);
                break;
            case 2:
                emulator->getCPU()->setTimerFrequency(emulator->getCPU()->getFrequencyCPU() / 64.0);
                break;
            case 3:
                emulator->getCPU()->setTimerFrequency(emulator->getCPU()->getFrequencyCPU() / 256.0);
                break;

        }
        return false;
    }
    if(address == LCDC && (byte & 0x80) && !(memory[LCDC] & 0x80) ){
        emulator->getDisplay()->clearWindow();
        memory[LY] = 0;
        emulator->getPPU()->setModePPU(HBLANK);
        emulator->getPPU()->clearCycles();
        memory[STAT] = (memory[STAT] & 0b11111100) | 0b00;
    }
    else if(address == LCDC && !(byte & 0x80) && (memory[LCDC] & 0x80)){
        emulator->getDisplay()->clearWindow();
        memory[LY] = 0;
        emulator->getPPU()->setModePPU(HBLANK);
        emulator->getPPU()->clearCycles();
        memory[STAT] = (memory[STAT] & 0b11111100) | 0b00;
    }
    if (address == IE) {
        memory[address] = byte;
        return false;
    }
    //  FIXME: General catch-all
    if (address >= 0xFF00 && address <= 0xFF7F) {
        memory[address] = byte;
        return false;
    }

    //  HRAM
    if (address >= 0xFF80 && address <= 0xFFFE){
        memory[address] = byte;
        return false;
    }

    ASSERT_NOT_REACHED("Failed to redirect write RAM");
}

/*
 *  A wrapper to write() for writing 16-bit values into memory
 */
void RAM::write16(uint16_t address, uint16_t value){
    write(address, (value & 0xFF00) >> 8);
    write(address+1, (value & 0xFF));
}

/*
 *  Insert a block with size 'length' of data from vector 'src' starting at the address 'dest'
 *  'override' ignores out-of-bound checks
 *  'offset' allows adding an offset to the source vector index
 */
bool RAM::insert(std::vector<uint8_t> src, uint16_t dest, size_t length, size_t offset, bool override){
    if( (src.empty() || dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF) && !override){
        return false;
    }

    for(size_t i = 0; i < length; i++){
        memory[dest + i] = src[offset + i];
    }
    return true;
}

/*
 *  Insert a block with size 'length' of data from array pointed to by 'src' at the address 'dest'
 *  'override' ignores out-of-bound checks
 *  'offset' allows adding an offset to the source vector index
 *  'dataLength' specifies the length of the source data buffer
 */
bool RAM::insert(const uint8_t* src, uint16_t dest, size_t length, size_t offset, size_t dataLength, bool override){
    if( (dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF) && !override){
        return false;
    }

    for(size_t i = 0; i < length; i++){
        if(offset + i >= dataLength){
            emulator->getDebugger()->emuLog("RAM::insert(pointer, 0x" + Utils::decHex(dest) + ", " + std::to_string(length) + ", " + std::to_string(offset) + " , " + std::to_string(dataLength) + ") failed!");
            emulator->getDebugger()->emuLog("Dump: i=" + std::to_string(i) + ", offset+i=" + std::to_string(offset+i));
        }
        assert(offset + i < dataLength);
        memory[dest + i] = src[offset + i];
    }
    return true;
}


/*
 *  Decodes the ROM header at 0x100 and sets memory settings
 */
void RAM::decodeHeader(){
    auto Debug = emulator->getDebugger();

    Debug->emuLog(" ########## App Header ##########");
    //  The rom doesn't have a header
    if(romFile.size() < 0x150){
        Debug->emuLog("Malformed ROM detected! Invalid size (expected romSize >= 0x150, got " + std::to_string(romFile.size()) + ")", LOGLEVEL::ERR);
        emulator->die("Corrupted ROM");
        return;
    }

    std::string title;
    for(int i = 0x0134; i <= 0x0143; i++){
        if(romFile[i] != 0){
            title += char(romFile[i]);
        }
    }
    Debug->emuLog("Title: " + title);

    std::string type;
    if(romFile[0x0143] == 0xC0) type = "GBC Only";
    else if(romFile[0x0143] == 0x80) type = "GB/GBC";
    else type = "Unknown";
    Debug->emuLog("Console type: " + type);


    //  Check and set the type of MBC used
    mbcType = (MBC_Type)romFile[0x0147];
    Debug->emuLog("MBC Type: " + controllerTypeLabel[mbcType]);

    size_t romBankCount = (2 << (romFile[0x0148]));
    Debug->emuLog("ROM Size: " + std::to_string(romBankCount) + " banks, " + std::to_string(32*(1 << romFile[0x0148])) + "kb");
    Debug->emuLog("ROM Version: " + std::to_string(romFile[0x014C]));

    unsigned int ramSize = 0;
    switch(romFile[0x0149]){
        case 0: ramSize = 0; break;
        case 1: ramSize = 2048; break;
        case 2: ramSize = 8192; break;
        case 3: ramSize = 32768; break;
        case 4: ramSize = 131072; break;
        case 5: ramSize = 65536; break;
        default: break;
    }
    size_t flashBankCount = ramSize / 8192;
    Debug->emuLog("ExtRAM Size: " + ((ramSize == 0)
                                    ? "None"
                                    : std::to_string(ramSize) + " bytes, " + std::to_string(flashBankCount) + " banks."));

    //  Create an MBC object
    switch(mbcType){
        case ROM: {
            //  Bind a dummy MBC
            mbc = std::make_unique<class ROM>((MBCFlags)0, romFile, flash, *Debug, romBankCount, 0);
            break;
        }
        case MBC1:
        case MBC1_RAM:
        case MBC1_RAM_BAT:{
            MBCFlags flags = MBC_MBC1;
            if(mbcType == MBC1_RAM_BAT) flags |= MBC_Battery;
            if(mbcType == MBC1_RAM || mbcType == MBC1_RAM_BAT) flags |= MBC_Flash;

            mbc = std::make_unique<class MBC1>(flags, romFile, flash, *emulator->getDebugger(), romBankCount, flashBankCount);
            break;
        }
        case MBC3:
        case MBC3_RAM:
        case MBC3_RAM_BAT:
        case MBC3_RAM_TIM_BAT:
        case MBC3_TIM_BAT: {
            MBCFlags flags = MBC_MBC3;
            if(mbcType == MBC3_RAM_BAT || mbcType == MBC3_RAM_TIM_BAT || mbcType == MBC3_TIM_BAT) flags |= MBC_Battery;
            if(mbcType == MBC3_RAM || mbcType == MBC3_RAM_BAT || mbcType == MBC3_TIM_BAT) flags |= MBC_Flash;
            if(mbcType == MBC3_TIM_BAT || mbcType == MBC3_RAM_TIM_BAT) flags |= MBC_Timer;

            mbc = std::make_unique<class MBC3>(flags, romFile, flash, *emulator->getDebugger(), romBankCount, flashBankCount);
            break;
        }
        case MBC5:
        case MBC5_RAM:
        case MBC5_RAM_BAT:
        case MBC5_RAM_BAT_RUMBLE:
        case MBC5_RAM_RUMBLE:
        case MBC5_RUMBLE:{
            MBCFlags flags = MBC_MBC5;
            if(mbcType != MBC5 && mbcType != MBC5_RUMBLE) flags |= MBC_Flash;
            if(mbcType == MBC5_RAM_BAT || mbcType == MBC5_RAM_BAT_RUMBLE) flags |= MBC_Battery;
            if(mbcType == MBC5_RAM_BAT_RUMBLE || mbcType == MBC5_RAM_RUMBLE || mbcType == MBC5_RUMBLE) flags |= MBC_Rumble;

            mbc = std::make_unique<class MBC5>(flags, romFile, flash, *emulator->getDebugger(), romBankCount, flashBankCount);
            break;
        }
        default:
            Debug->emuLog("Error creating an MBC object! Type not implemented", LOGLEVEL::ERR);
            emulator->die("Unimplemented MBC");
            break;
    }

    std::string features;
    if(mbc->supports(MBC_MBC1)) features += "MBC1 ";
    else if(mbc->supports(MBC_MBC2)) features += "MBC2 ";
    else if(mbc->supports(MBC_MBC3)) features += "MBC3 ";
    else if(mbc->supports(MBC_MBC5)) features += "MBC5 ";

    if(mbc->supports(MBC_Flash)) features += "Flash ";
    if(mbc->supports(MBC_Timer)) features += "Timer ";
    if(mbc->supports(MBC_Battery)) features += "Battery ";
    if(mbc->supports(MBC_Rumble)) features += "Rumble ";
    Debug->emuLog("MBC features: " + features);

    unsigned char sum = 0;
    for(int i = 0x0134; i <= 0x014C; i++){
        sum = sum - romFile[i] - 1;
    }
    Debug->emuLog("Header checksum: " + Utils::decHex(romFile[0x014D],2) + "/" + Utils::decHex(sum,2) + ((romFile[0x014D] == (sum & 0xFF)) ? " [VALID]" : " [INVALID]"));
    Debug->emuLog(" ########## End Header ##########");
}

/*
 *  Empty RAM
 */
void RAM::clear(){
    memory.clear();
    memory.resize(0x10000);
}

/*
 *  Reload RAM (Fresh start)
 */
void RAM::reload(){
    clear();
    mountBanksRAM();
    memory[P1] = 0xC0;
    memory[LCDC] = 0x91;
}

/*
 *  Mounts the ROM in RAM
 */
void RAM::mountBanksRAM(){
    //  Clear RAM contents first
    clear();

    if(mbcType != ROM){
        emulator->getDebugger()->emuLog("MBC object check: ");
        std::string val  = "flashBankCount: " + std::to_string(mbc->getFlashBankCount()) + ",  flashSize: " + std::to_string(mbc->getFlashSize());
        std::string val2 = "  romBankCount: " + std::to_string(mbc->getROMBankCount()) + ",    romSize: " + std::to_string(romFile.size());
        emulator->getDebugger()->emuLog(val);
        emulator->getDebugger()->emuLog(val2);
    }

    //  If an MBC is in use,
    //  Check if the flash is present on disk and load it
    if(mbc->supports(MBCFlags_::MBC_Flash)){
        if(Utils::exists(emulator->getConfig()->getSavename())){
            emulator->getDebugger()->emuLog("Loading flash from save.");
            flash = Utils::loadFromFile(emulator->getConfig()->getSavename());
            if(flash.size() != mbc->getFlashSize()){
                emulator->getDebugger()->emuLog("External flash size does not match header-specified Flash size! Skipping..", LOGLEVEL::ERR);
                flash.clear();
                flash.resize(mbc->getFlashSize());
            }
        } else {
            emulator->getDebugger()->emuLog("Save file does not exist.");
            flash.resize(mbc->getFlashSize());
        }
    }

    //  MBC-based behavior
    switch(mbcType){
        case ROM:
            insert(romFile, 0x0, romFile.size(), 0);
            break;

        case MBC1:
        case MBC1_RAM:
        case MBC1_RAM_BAT:
        case MBC3:
        case MBC3_RAM:
        case MBC3_RAM_BAT:
        case MBC3_RAM_TIM_BAT:
        case MBC3_TIM_BAT:
        case MBC5:
        case MBC5_RAM:
        case MBC5_RAM_BAT:
        case MBC5_RAM_BAT_RUMBLE:
        case MBC5_RAM_RUMBLE:
        case MBC5_RUMBLE:
            mbc->mountBanks();
            break;

        default:
            emulator->getDebugger()->emuLog("Unimplemented MBC type of " + controllerTypeLabel[mbcType] + "!", LOGLEVEL::ERR);
            emulator->halt();
            break;
    }
}

/*
 *  Saves the contents of flash to disk
 */
void RAM::unmountRAM(){
    if(mbc->getFlashSize() > 0){
        std::ofstream save;
        save.open(emulator->getConfig()->getSavename(), std::ios::binary);
        save.write((char*)&flash[0], mbc->getFlashSize());
        save.close();
    }
}

/*
 *  Returns base vector pointer to the memory array
 */
uint8_t* RAM::getBaseMemoryPointer(){
    return &memory[0];
}

/*
 *  Loads the ROM specified by 'path' into a ROM buffer.
 *  This does not actually mount the ROM
 */
void RAM::loadROM(std::string path){
    romFile = Utils::loadFromFile(path);
}

/*
 *  Returns ROM file size (in bytes)
 */
size_t RAM::getSizeROM(){
    return romFile.size();
}

/*
 *  Returns the base pointer to the flash vector
 */
uint8_t* RAM::getFlashBasePointer(){
    return &flash[0];
}

/*
 *  Returns the base pointer to the ROM file vector
 */
uint8_t* RAM::getROMBasePointer() {
    return &romFile[0];
}

/*
 *  Returns the size of the flash vector
 */
size_t RAM::getSizeFlash() {
    return flash.size();
}

void RAM::beginTransferOAM(uint16_t source) {
//    if(emulator->getPPU()->getPPUMode() != PPU_MODE::VBLANK){
//        std::string mode;
//        switch(emulator->getPPU()->getPPUMode()){
//            case PPU_MODE::HBLANK:
//                mode = "HBlank";
//                break;
//            case PPU_MODE::OAM:
//                mode = "OAM";
//                break;
//            case PPU_MODE::PIXTX:
//                mode = "Pixel Transfer (wtf?)";
//                break;
//        }
//        emulator->getDebugger()->emuLog("Tried triggering DMA transfer from invalid PPU mode! (" + mode + ")", LOGLEVEL::WARN);
//    }

    if(!CurrentOAMTransfer.completed){
        emulator->getDebugger()->emuLog("Tried triggering an OAM transfer while a transfer was already in progress", LOGLEVEL::WARN);
        return;
    }
    if(source % 0x100 != 0){
        emulator->getDebugger()->emuLog("Tried triggering an OAM transfer from an invalid address (" + Utils::decHex(source) + ")", LOGLEVEL::WARN);
        return;
    }

    //  TODO: Log transfers for cool debug features
    CurrentOAMTransfer.completed = false;
    CurrentOAMTransfer.source = source;
    CurrentOAMTransfer.nextByte = 0;
}

/*
 *  Updates memory transfers, currently only OAM
 */
void RAM::updateTransfers(size_t cycles) {
    if(CurrentOAMTransfer.completed) return;
    //  Increment internal cycle counter
    CurrentOAMTransfer.clockCounter += cycles;
    while(CurrentOAMTransfer.nextByte < OAM_TRANSFER_SIZE && CurrentOAMTransfer.clockCounter >= OAM_TRANSFER_CYCLES_PER_BYTE){
        memory[OAM_TRANSFER_TARGET + CurrentOAMTransfer.nextByte] = memory[CurrentOAMTransfer.source + CurrentOAMTransfer.nextByte];
        CurrentOAMTransfer.nextByte++;
        CurrentOAMTransfer.clockCounter -= OAM_TRANSFER_CYCLES_PER_BYTE;
    }
    //  OAM completed
    if(CurrentOAMTransfer.nextByte == OAM_TRANSFER_SIZE){
        CurrentOAMTransfer.completed = true;
    }
}
