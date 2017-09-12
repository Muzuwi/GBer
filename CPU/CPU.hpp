#include <vector>

#pragma once
namespace CPU{
	struct Registers{
		extern char16_t SP, PC, BC, DE, HL;	// The BC, DE, HL might be redundant
		extern char A, F, B, C, D, E, H, L;
	};

	extern std::vector<char16_t> Stack;
	bool cycle();

}