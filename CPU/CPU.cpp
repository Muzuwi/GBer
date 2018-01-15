#include <vector>
#include <iostream>
#include "CPU.hpp"
#include "../Math/Math.hpp"
#include "../Memory/RAM.hpp"

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
			//  Use normal op table
		} else {
			//
		}


		return true;
	}
}  // namespace CPU
