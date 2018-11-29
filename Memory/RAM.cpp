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
	std::vector<unsigned char> romFile;
	std::vector<unsigned char> bootRom;
	std::vector<unsigned char> RAM(0x10000);

	/*
		Reads a byte from RAM
		Arguments: memory address to read from
	*/
	unsigned char read(char16_t address){
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
		for(Debug::MemoryBreakpoint br : Debug::memoryBreakpoints){
			if(br.addr == address && br.r){
				Debug::menuText = "Memory read at $" + Math::decHex(address);
				Debug::timer = 180;
				CPU::step = false;
				CPU::continueExec = false;
			}
		}
	}

	/*
		Write byte to RAM
		Arguments: Address, byte to write
	*/
	bool write(char16_t address, unsigned char byte){
		if(address > 0xFFFF || address < 0x0){
			Debug::emuLog("Failed writing " + Math::decHex(byte) + " to address " + Math::decHex(address), Debug::LEVEL::ERR);
			return false;
		}else if( ((address >= 0x8000 && address <= 0x9FFF) || (address >= 0xFE00 && address <= 0xFE9F)) && PPU::currentPpuMode == PPU::PPU_MODE::PIXTX){   //  Accessing VRAM during pixel transfer
			Debug::emuLog("Tried accessing VRAM during Pixel Transfer", Debug::LEVEL::WARN);
			return false;
		}else if( (address >= 0xFE00 && address <= 0xFE9F) && PPU::currentPpuMode == PPU::PPU_MODE::OAM){
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
		if(address == 0xFF42){
			PPU::SCY = byte;
		} else if(address == 0xFF43){
			PPU::SCX = byte;
		}

		if(address == 0xFF40){
			bool control = byte & 0x80;
			bool displayW = byte & 0x20;
			bool displayO = byte & 0x2;
			bool displayBW = byte & 0x1;
			int w = 8, h = (byte & 0x4) ? 16 : 8;
			PPU::LCDC.lcdEnabled = control;
			PPU::LCDC.objDisplay = displayO;
			PPU::LCDC.windowDisplay = displayW;
			PPU::LCDC.windowBgDisplay = displayBW;
			PPU::LCDC.objW = w;
			PPU::LCDC.objH = h;
		}


		for(Debug::MemoryBreakpoint br : Debug::memoryBreakpoints){
			if(br.addr == address && br.w){
				Debug::menuText = "Memory write at $" + Math::decHex(address);
				Debug::timer = 180;
				CPU::step = false;
				CPU::continueExec = false;
			}
		}
		return true;
	}

	/*
		Writes 2 bytes to RAM
		Arguments: address, byte sequence
	*/
	bool write_16(char16_t address, char16_t byte){
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
		if( (in.size() == 0 || dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF) && !over){
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

}  // namespace RAM
