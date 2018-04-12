#pragma once
#include <stack>

namespace CPU{
	struct Reg{
	    char16_t SP, PC;
	    unsigned char A, F, B, C, D, E, H, L;
	};
	extern Reg Registers;
	extern bool halt;
	extern std::stack<char16_t> GBStack;
	void start();
	bool cycle();
	void clearState();
	inline char16_t readHL();
	inline char16_t readBC();
	inline char16_t readDE();
	inline char16_t readAF();
	inline void writeHL(char16_t);
	inline void writeBC(char16_t);
	inline void writeDE(char16_t);
	inline void writeAF(char16_t);
	inline void copyToC(int);
}  // namespace CPU
