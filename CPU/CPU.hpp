#pragma once
#include <vector>

namespace CPU{
	struct Reg{
	    char16_t SP, PC;
	    unsigned char A, F, B, C, D, E, H, L;
	};
	extern Reg Registers;
	extern bool halt;
	extern std::vector<char16_t> Stack;
	void start();
	bool cycle();
	inline char16_t readHL();
	inline char16_t readBC();
	inline char16_t readDE();
	inline void writeHL(char16_t);
	inline void writeBC(char16_t);
	inline void writeDE(char16_t);
	inline void copyToC(int);
}  // namespace CPU
