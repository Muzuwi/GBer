#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include "RAM.hpp"
#include "../Math/Math.hpp"
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
	std::vector<unsigned char> RAM(0x10000);

	/*
		Reads a byte from RAM
		Arguments: memory address to read from
	*/
	unsigned char read(char16_t address){
		if(address > 0xFFFF || address < 0x0){
			// Most likely unspecified
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
		if(address > 0xFFFF || address < 0x0){
			return false;
		}else if( (address >= 0xC000) && (address <= 0xDE00) ){		//  Echo RAM
			RAM[address] = byte;
			RAM[address + 0x2000] = byte;
		}else if( (address >= 0xE000) && (address <= 0xFE00) ){
			RAM[address] = byte;
			RAM[address - 0x2000] = byte;
		}else{
			RAM[address] = byte;
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
			RAM[address - 0x2000 - 0x01] = lower;
		}else{
			RAM[address] = byte;
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
	bool insert(std::vector<unsigned char> in, char16_t dest, unsigned int length, bool override){
		if( (in.size() == 0 || dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF) && !override){
			return false;
		}

		for(size_t i = 0; i < length; i++){
			RAM[dest + i] = in[i];
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
}  // namespace RAM
