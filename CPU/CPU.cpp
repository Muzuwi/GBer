#include <vector>

namespace CPU{
	struct Registers{
		char16_t SP, PC, BC, DE, HL;
		char A, F, B, C, D, E, H, L;
	};

	std::vector<char16_t> Stack;

	bool cycle(){

	}
	
}