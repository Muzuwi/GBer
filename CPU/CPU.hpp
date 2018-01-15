#pragma once
#include <vector>

namespace CPU{
	struct Reg{
	    char16_t SP, PC, BC, DE, HL;	  //  The BC, DE, HL might be redundant
	    char A, F, B, C, D, E, H, L;
	};
	extern Reg Registers;
	extern bool halt;
	extern std::vector<char16_t> Stack;
	void start();
	bool cycle();
}  // namespace CPU
