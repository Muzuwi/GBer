#include "RAM.hpp"
#include <fstream>
#include <map>
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <iostream>
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

	std::vector<unsigned char>RAM(0x10000);

	/*
		Reads a byte from RAM
		Arguments: memory address to read from
	*/
	char read(char16_t address){
		if(address > 0xFFFF || address < 0x0){
			// Most likely unspecified
			return 0xFF;
		}else{
			return RAM[address];
		}

	}


	/*
		Reads a sequence of bytes from RAM
		Arguments: memory address to read from, byte count

		DOESN'T ACTUALLY WORK FOR 2+ BYTES
	*/

	char16_t readSeq(char16_t address, unsigned int count){
		if(address > 0xFFFF || address < 0x0 || address + count > 0xFFFF){
			// Most likely unspecified
			return 0xFF;
		}else{
			char16_t op = 0;
			int mult = 0x100, pos = 0;
			unsigned int pow = 1;

			while(pow + 1 < count){
				mult *= 0x100;
				pow++;
			}

			do{
				std::cout << "\n---" << Math::decHex(RAM[address + pos]) << "---\n";
				op += RAM[address + pos]*mult;
				pos++;
				mult /= 0x100;
				std::cout << "\n" << Math::decHex(op) << " " << mult << " \n";
			}while(pos < count);
			return op;
		}

	}
	/*
		Write byte to RAM
		Arguments: Address, byte to write
	*/
	bool write(char16_t address, unsigned char byte){
		if(address > 0xFFFF || address < 0x0){
			return false;
		}else{ 
			RAM[address] = byte;
			return true;
		}
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
				RAM[source + i] = 0x0;	// Not sure if this should be 0x0, or 0xFF
			}
			return true;
		}
	}

	/*
		Insert the contents of a vector into the RAM (For loading the ROM, bootrom, whatever)
		Arguments: Vector<char>, Destination address, length of block to insert
	*/
	bool insert(std::vector<unsigned char> in, char16_t dest, unsigned int length){
		if(in.size() == 0 || dest > 0xFFFF || dest < 0x0 || (dest + length) > 0xFFFF){
			return false;
		}

		for(unsigned int i = 0; i < length; i++){
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
			if(i % 2 == 0 ){
				file << " ";
			}
			file << Math::decHex(RAM[i]);
		}
		file.close();
		return true;
	}
}