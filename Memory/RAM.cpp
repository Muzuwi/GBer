#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include "../Debug/Debug.hpp"
#include "RAM.hpp"
#include "../Math/Math.hpp"
#include "../CPU/CPU.hpp"
#include "../CPU/PPU.hpp"

namespace RAM{
	/* ROM0(0x4000);		// Bank #0
	   SROM(0x4000);		// Switchable ROM bank
	   VRAM(0x2000);		// VRAM
	   SRAM(0x2000);		// Switchable RAM bank ( i think for cartridges?)
	   MRAM(0x2000);		// Main RAM
	   ERAM(0x1E00);		// Echo RAM
	    OAM(0x00A0);		// OAM
	   EMP0(0x0060);		// Empty #0 
	   	 IO(0x004C);			// I/O 
	   EMP1(0x0034);		// Empty #1 
	   IRAM(0x0080);		// Internal RAM */
	MBC_Type controllerType;
	std::vector<unsigned char> romFile;
	std::vector<unsigned char> bootRom;
	std::vector<unsigned char> RAM(0x10000);
	BankingMode currentBankingMode = ROM_BANK;
	int mountedBankNumber;
	bool ramEnabled;

	//  Mapper labels
	std::map<MBC_Type, std::string> controllerTypeLabel = {
			{ROM , "ROM"},
			{MBC1 , "MBC1"},
			{MBC1_RAM , "MBC1_RAM"},
			{MBC1_RAM_BAT , "MBC1_RAM_BAT"},
			{MBC2 , "MBC2"},
			{MBC2_BAT , "MBC2_BAT"},
			{ROM_RAM , "ROM_RAM"},
			{ROM_RAM_BAT , "ROM_RAM_BAT"},
			{MMM01 , "MMM01"},
			{MMM01_RAM , "MMM01_RAM"},
			{MMM01_RAM_BAT , "MMM01_RAM_BAT"},
			{MBC3_TIM_BAT , "MBC3_TIM_BAT"},
			{MBC3_RAM_TIM_BAT , "MBC3_RAM_TIM_BAT"},
			{MBC3 , "MBC3"},
			{MBC3_RAM , "MBC3_RAM"},
			{MBC3_RAM_BAT , "MBC3_RAM_BAT"},
			{MBC5 , "MBC5"},
			{MBC5_RAM , "MBC5_RAM"},
			{MBC5_RAM_BAT , "MBC5_RAM_BAT"},
			{MBC5_RUMBLE , "MBC5_RUMBLE"},
			{MBC5_RAM_RUMBLE , "MBC5_RAM_RUMBLE"},
			{MBC5_RAM_BAT_RUMBLE , "MBC5_RAM_BAT_RUMBLE"},
			{MBC6_RAM_BAT , "MBC6_RAM_BAT"},
			{MBC7_RAM_BAT_ACCEL , "MBC7_RAM_BAT_ACCEL"},
			{POCKET_CAM , "POCKET_CAM"},
			{TAMA5 , "TAMA5"},
			{HUC3 , "HUC3"},
			{HUC1_RAM_BAT , "HUC1_RAM_BAT"}
	};


	/*
		Reads a byte from RAM
		Arguments: memory address to read from
	*/
	unsigned char read(char16_t address){

		bool canRead = handleMBCread(address);

        for(Debug::MemoryBreakpoint br : Debug::memoryBreakpoints){
            if(br.addr == address && br.r){
                Debug::menuText = "Memory read at $" + Math::decHex(address);
                Debug::timer = 180;
                CPU::step = false;
                CPU::continueExec = false;
            }
        }

        if(canRead){
        	Debug::emuLog("Trying to read from write-only memory", Debug::LEVEL::WARN);
        }

        if(address > 0xFFFF || address < 0x0){
			// Most likely unspecified
			Debug::emuLog("Reading from undefined address " + Math::decHex(address), Debug::LEVEL::ERR);
			return 0xFF;
		}else if( ((address >= 0x8000 && address <= 0x9FFF) || (address >= 0xFE00 && address <= 0xFE9F)) && PPU::currentPpuMode == PPU::PPU_MODE::PIXTX){   //  Accessing VRAM during pixel transfer
			Debug::emuLog("Tried accessing VRAM during Pixel Transfer", Debug::LEVEL::WARN);
			return 0xFF;
		}else if( (address >= 0xFE00 && address <= 0xFE9F) && PPU::currentPpuMode == PPU::PPU_MODE::OAM){
			Debug::emuLog("Tried accessing OAM RAM during OAM Search", Debug::LEVEL::WARN);
			return 0xFF;
		}else{
			return RAM[address];
		}
	}

	/*
		Write byte to RAM
		Arguments: Address, byte to write
	*/
	bool write(char16_t address, unsigned char byte){

		for(Debug::MemoryBreakpoint br : Debug::memoryBreakpoints){
			if(br.addr == address && br.w){
				Debug::menuText = "Memory write at $" + Math::decHex(address);
				Debug::timer = 180;
				CPU::step = false;
				CPU::continueExec = false;
			}
		}

		bool canWrite = handleMBCwrite(address, byte);

		if(canWrite){
			if(address > 0xFFFF || address < 0x0){
				Debug::emuLog("Failed writing " + Math::decHex(byte) + " to address " + Math::decHex(address), Debug::LEVEL::ERR);
				return false;
			}else if( (address >= 0x8000 && address <= 0x9FFF) && PPU::currentPpuMode == PPU::PPU_MODE::PIXTX){   //  Accessing VRAM during pixel transfer
				Debug::emuLog("Tried accessing VRAM during Pixel Transfer", Debug::LEVEL::WARN);
				return false;
			}else if( (address >= 0xFE00 && address <= 0xFE9F) && (PPU::currentPpuMode == PPU::PPU_MODE::HBLANK || PPU::currentPpuMode == PPU::VBLANK)){
				Debug::emuLog("Tried accessing OAM RAM during OAM Search", Debug::LEVEL::WARN);
				return false;
			}else if( (address >= 0xC000) && (address <= 0xDE00) ){		//  Echo RAM
				RAM[address] = byte;
				RAM[address + 0x2000] = byte;
			}else if( (address >= 0xE000) && (address <= 0xFE00) ){
				RAM[address] = byte;
				RAM[address - 0x2000] = byte;
			}else{
				if(address == 0xFF04 || address == 0xFF44){
					RAM[address] = 0;
				} else {
					RAM[address] = byte;
				}
			}
		}

		if(address == 0xFF41){
			if(byte & 0x40) {
				PPU::intHBL = true;
			}
			if(byte & 0x20) {
				PPU::intVBL = true;
			}
			if(byte & 0x10) {
				PPU::intOAM = true;
			}
			if(byte & 0x08) {
				PPU::intLYC = true;
				PPU::LYC_eq = (bool)(byte & 4);
			}
		}

		if(address == 0xFF46){
			// TODO: This might screw up if ppu is in incorrect mode
			if(byte > 0 && byte <= 0xF1){
				Debug::emuLog("OAM transfer from " + Math::decHex(byte), Debug::LEVEL::INFO);
				for(int i = 0; i < 140; i++){
					RAM::RAM[0xFE00 + i] = RAM::RAM[byte*0x100 + i];
				}
			}
		}

		if (address == 0xFF02 && byte == 0x81) {
			std::cout << read(0xFF01);
		}

		if (address == 0xFF07){
			switch(byte & 3){
				case 0:
					CPU::timerFreq = CPU::freq / 1024;
					break;
				case 1:
					CPU::timerFreq = CPU::freq / 16;
					break;
				case 2:
					CPU::timerFreq = CPU::freq / 64;
					break;
				case 3:
					CPU::timerFreq = CPU::freq / 256;
					break;

			}
		}
		return true;
	}

	/*
		Writes 2 bytes to RAM
		Arguments: address, byte sequence
	*/
	bool write_16(char16_t address, char16_t byte){
		// TODO: Add support for banking
		char upper = (byte >> 8), lower = (byte & ~0xFF00);
		if(address > 0xFFFF - 0x01 || address < 0x0){
			Debug::emuLog("Failed writing " + Math::decHex(byte) + " to address " + Math::decHex(address), Debug::LEVEL::ERR);
			return false;
		}else if( (address >= 0xC000) && (address <= 0xDE00 - 0x01) ){		//  Echo RAM
			RAM[address] = upper;
			RAM[address + 0x01] = lower;

			RAM[address + 0x2000] = upper;
			RAM[address + 0x2000 + 0x01] = lower;
		}else if( (address >= 0xE000) && (address <= 0xFE00 - 0x01) ){
			RAM[address] = upper;
			RAM[address + 0x01] = lower;

			RAM[address - 0x2000] = upper;
			RAM[address - 0x2000 + 0x01] = lower;
		}else{
			RAM[address] = upper;
			RAM[address + 0x01] = lower;
		}
		for(Debug::MemoryBreakpoint br : Debug::memoryBreakpoints){
			if( (br.addr == address || br.addr == address + 1) && br.w){
				Debug::emuLog("Memory");
				if(br.addr == address) Debug::menuText = "Memory write at $" + Math::decHex(address);
				if(br.addr == address + 1) Debug::menuText = "Memory write at $" + Math::decHex(address+1);
				Debug::timer = 180;
				CPU::step = false;
				CPU::continueExec = false;
			}
		}

		return true;
	}




	/*
		Copy a block of bytes to another location in RAM
		Arguments: Source address, Destination address, length of block to copy
	*/	
	bool copy(char16_t source, char16_t dest, unsigned int length){
		if(source > 0xFFFF || source < 0x0 || dest > 0xFFFF || dest < 0x0 || length < 0x0 ||
		  (source + length) > 0xFFFF || (dest + length) > 0xFFFF ){
				return false;
		}else{
			for(unsigned int i = 0; i <= length; i++){
				RAM[dest + i] = RAM[source + i];
			}
			return true;
		}
	}

	/*
		Move a block of bytes to another location in RAM
		Arguments: Source address, Destination address, length of block to move
	*/	
	bool move(char16_t source, char16_t dest, unsigned int length){
			if(source > 0xFFFF || source < 0x0 || dest > 0xFFFF || dest < 0x0 ||
		  (source + length) > 0xFFFF || (dest + length) > 0xFFFF ){
				return false;
		}else{
			for(unsigned int i = 0; i <= length; i++){
				RAM[dest + i] = RAM[source + i];
				RAM[source + i] = 0x0;	    // Not sure if this should be 0x0, or 0xFF
			}
			return true;
		}
	}

	/*
		Insert the contents of a vector into the RAM (For loading the ROM, bootrom, whatever)
		Arguments: Vector<char>, Destination address, length of block to insert
		Override: ignores attempts to prevent out of bounds writes/whatever else is there
	*/
	bool insert(std::vector<unsigned char> in, char16_t dest, unsigned int length, int offset, bool over){
		if( (in.empty() || dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF) && !over){
			return false;
		}

		for(size_t i = 0; i < length; i++){
			RAM[dest + i] = in[offset + i];
		}
		return true;
	}

	/*
		Dumps the memory to a file
		Arguments: file name
	*/
	bool dump(std::string name){
		std::ofstream file;
		file.open(name, std::ios::binary);
		for(unsigned int i = 0; i < 0x10000; i++){
			if(i == 0){
				file << "$" << Math::decHex(i) << " | ";
			}
			if(i % 32 == 0 && i != 0){
				file << "\n";
				file << "$" << Math::decHex(i) << " | ";
			}
			if(i % 2 == 0){
				file << " ";
			}
			file << Math::decHex(RAM[i]);
		}
		file.close();
		return true;
	}

	/*
		Clears RAM
	*/
	void clear() {
		RAM.clear();
		RAM.resize(0x10000);
	}

	void decodeHeader(){
		Debug::emuLog(" ########## App Header ##########");
		std::string title;
		for(int i = 0x0134; i <= 0x0143; i++){
			if(RAM::RAM[i] != 0){
				title += char(RAM::RAM[i]);
			}
		}
		Debug::emuLog("Title: " + title);

		std::string type;
		if(RAM::RAM[0x0143] == 0xC0){
			type = "GBC Only";
		}else if(RAM::RAM[0x0143] == 0x80){
			type = "GB/GBC";
		}else{
			type = "Unknown";
		}
		Debug::emuLog("Console type: " + type);

		std::string mbcType;
		RAM::controllerType = (RAM::MBC_Type)RAM::RAM[0x0147];
		Debug::emuLog("MBC Type: " + RAM::controllerTypeLabel[RAM::controllerType]);

		Debug::emuLog("ROM Size: " + std::to_string(2 << (RAM::RAM[0x0148])) + " banks, " + std::to_string(32*(1 << RAM::RAM[0x0148])) + "kb");
		Debug::emuLog("ROM Version: " + std::to_string(RAM::RAM[0x014C]));

		unsigned char sum = 0;
		for(int i = 0x0134; i <= 0x014C; i++){
			sum = sum - RAM::RAM[i] - 1;
		}
		Debug::emuLog("Header checksum: " + Math::decHex(RAM::RAM[0x014D]) + "/" + Math::decHex(sum) + ((RAM::RAM[0x014D] == (sum & 0xFF)) ? " [VALID]" : " [INVALID]"));
	}

	void mountMemoryBanks(){
		if(controllerType == MBC_Type::MBC1 || controllerType == MBC_Type::MBC1_RAM || controllerType == MBC_Type::MBC1_RAM_BAT){
			//  Mount bank 00 and 01 (default)
			insert(romFile, 0x0, 0x4000, 0, false);
			insert(romFile, 0x4000, 0x4000, 0x4000, false);
			if(controllerType == MBC_Type::MBC1_RAM || controllerType == MBC_Type::MBC1_RAM_BAT){

			}
		} else if (controllerType == MBC2_BAT || controllerType == MBC2){
            //  Mount bank 00 and 01 (default)
            insert(romFile, 0x0, 0x4000, 0, false);
            insert(romFile, 0x4000, 0x4000, 0x4000, false);

		} else if (controllerType == MBC3 || controllerType == MBC3_RAM || controllerType == MBC3_RAM_BAT || controllerType == MBC3_RAM_TIM_BAT || controllerType == MBC3_TIM_BAT) {
            //  Mount bank 00 and 01 (default)
            insert(romFile, 0x0, 0x4000, 0, false);
            insert(romFile, 0x4000, 0x4000, 0x4000, false);
        } else if (controllerType == ROM){
		    insert(romFile, 0x0, 0x8000, 0, false);
		}

	}

	/*
	 * Handles memory bank reading
	 * Returns true if memory can't be accessed
	 */
	bool handleMBCread(char16_t addr){
		if(controllerType == ROM){
			return false;
		} else if(controllerType == MBC_Type::MBC1 || controllerType == MBC_Type::MBC1_RAM || controllerType == MBC_Type::MBC1_RAM_BAT){
			if(addr >= 0x0 && addr <= 0x7FFF){
				return false;
			} else if(addr >= 0xA000 && addr <= 0xBFFF){
				return !(controllerType == MBC1_RAM_BAT || controllerType == MBC1_RAM);
			} else {
			    return false;
			}
		}  else if (controllerType == MBC2 || controllerType == MBC2_BAT) {
            if(addr >= 0x0 && addr <= 0x7FFF){
                return false;
            } else if(addr >= 0xA000 && addr <= 0xA1FF){
                return !(controllerType == MBC2_BAT);
            } else {
                return false;
            }
        } else if (controllerType == MBC3_RAM || controllerType == MBC3_RAM_BAT || controllerType == MBC3 ||
                   controllerType == MBC3_RAM_TIM_BAT) {
            if(addr >= 0x0 && addr <= 0x7FFF){
                return false;
            } else if(addr >= 0xA000 && addr <= 0xBFFF){
                return !(controllerType == MBC3);
            } else {
                return false;
            }
        } else {
			std::cout << "YOU FUCK";
			return false;
		}
		//  This should never be reached
		assert(false);
		return false;

	}

	/*
	 * Handles memory bank writing
	 * Returns true if a write can occur in this location (If memory will be affected by the write)
	 */
	bool handleMBCwrite(char16_t addr, unsigned char byte) {
        if (controllerType == ROM) {
            return true;
        } else if (controllerType == MBC_Type::MBC1 || controllerType == MBC_Type::MBC1_RAM ||
                   controllerType == MBC_Type::MBC1_RAM_BAT) {
            if (addr >= 0 && addr <= 0x1FFF && (controllerType == MBC_Type::MBC1_RAM || controllerType == MBC_Type::MBC1_RAM_BAT)) {
                ramEnabled = (byte & 0xF) == 0xA;
                return false;
            } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                if (byte == 0x0) {
                    //  Remount 4000-7FFF to bank 01
                    insert(romFile, 0x4000, 0x4000, 0x4000, false);
                } else if (byte > 0 && byte <= 0x1F) {
                    mountedBankNumber = (mountedBankNumber & 0xE0) | byte;
                    insert(romFile, 0x4000, 0x4000, mountedBankNumber * 0x4000, false);
                    Debug::emuLog("MBC1: remounted to bank " + std::to_string(mountedBankNumber));
                }
                return false;
            } else if (addr >= 0x4000 && addr <= 0x5FFF) {
                if (currentBankingMode == ROM_BANK) {
                    mountedBankNumber = (mountedBankNumber & 0x1F) | byte;
                    insert(romFile, 0x4000, 0x4000, mountedBankNumber * 0x4000, false);
                    Debug::emuLog("MBC1: remounted to bank " + std::to_string(mountedBankNumber));
                } else if (currentBankingMode == RAM_BANK) {
                    // TODO
                }
                return false;
            } else if (addr >= 0x6000 && addr <= 0x7FFF) {
                if (byte == 0) {
                    currentBankingMode = ROM_BANK;
                } else if (byte == 1) {
                    currentBankingMode = RAM_BANK;
                }
                return false;
            } else {
                return true;
            }
        } else if (controllerType == MBC2 || controllerType == MBC2_BAT) {
            if (addr >= 0 && addr <= 0x1FFF) {
                if( ((addr >> 4) & 0x1) == 0 ){
                    ramEnabled = !ramEnabled;
                }
                return false;
            } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                unsigned char bankSelect = byte & 0xF0;
                mountedBankNumber = bankSelect;
                insert(romFile, 0x4000, 0x4000, mountedBankNumber * 0x4000, false);
                Debug::emuLog("MBC2: remounted to bank " + std::to_string(mountedBankNumber));
                return false;
            } else {
                return true;
            }
        } else if (controllerType == MBC3_RAM || controllerType == MBC3_RAM_BAT || controllerType == MBC3 ||
                   controllerType == MBC3_RAM_TIM_BAT || controllerType == MBC3_TIM_BAT) {
            if (addr >= 0 && addr <= 0x1FFF && (controllerType != MBC3)) {
                if ((byte & 0xF) == 0xA) {
                    ramEnabled = true;
                } else {
                    ramEnabled = false;
                }
                return false;
            } else if ( addr >= 0xA000 && addr <= 0xBFFF){
                if (currentBankingMode == RAM_BANK){
                    //  TODO
                } else if (currentBankingMode == RTC_BANK){
                    //  TODO
                }
                return false;
            } else if (addr >= 0x2000 && addr <= 0x3FFF) {
                if (byte == 0x0) {
                    //  Remount 4000-7FFF to bank 01
                    insert(romFile, 0x4000, 0x4000, 0x4000, false);
                } else if (byte > 0 && byte <= 0x7F) {
                    mountedBankNumber = (mountedBankNumber & 0xE0) | byte;
                    insert(romFile, 0x4000, 0x4000, mountedBankNumber * 0x4000, false);
                    Debug::emuLog("MBC3: remounted to bank " + std::to_string(mountedBankNumber));
                }
                return false;
            } else if (addr >= 0x4000 && addr <= 0x5FFF) {
                if(byte >= 0 && byte <= 7){
                    currentBankingMode = RAM_BANK;
                } else if (byte >= 8 && byte <= 0xC){
                    currentBankingMode = RTC_BANK;
                }
                return false;
            } else if (addr >= 0x6000 && addr <= 0x7FFF) {
                //  TODO
                return false;
            } else {
                return true;
            }
        } else {
		    return false;
		}


		//  This should never be reached
		assert(false);
		return false;
	}
}  // namespace RAM
