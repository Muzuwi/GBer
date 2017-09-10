#include "RAM.hpp"
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

	std::vector<char> RAM(0x10000);

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
		Write byte to RAM
		Arguments: Address, byte to write
	*/
	bool write(char16_t address, char byte){
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
			for(int i = 0; i <= length; i++){
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
		if(source > 0xFFFF || source < 0x0 || dest > 0xFFFF || dest < 0x0 || length < 0x0 ||
		  (source + length) > 0xFFFF || (dest + length) > 0xFFFF ){
				return false;
		}else{
			for(int i = 0; i <= length; i++){
				RAM[dest + i] = RAM[source + i];
				RAM[source + i] = 0x0;	// Not sure if this should be 0x0, or 0xFF
			}
			return true;
		}

	}
}