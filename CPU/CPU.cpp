#include <vector>
#include "CPU.hpp"

namespace CPU{
	Reg Registers;
	bool halt = false;
	std::vector<char16_t> Stack;

	void start(){
		while(!halt){
			cycle();
		}

	}

	bool cycle(){
		unsigned char prefix = RAM::read(Registers.PC), 
					  opcode = RAM::read(Registers.PC + 0x001), 
					  imm1 = RAM::read(Registers.PC + 0x002), imm2 = RAM::read(Registers.PC + 0x003);
		if(prefix != 0xCB){
			// Use normal op table
			switch(prefix){
				case 0x00: OPCodes::gb00(); break;



				default: std::cout << "!!!Unknown opcode!!! " << Math::decHex(prefix) << "\n";
			}
		}else{
			// Use cb op table



		}






		return true;
	}
}