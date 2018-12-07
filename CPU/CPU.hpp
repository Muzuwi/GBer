#pragma once
#include <stack>
#include <chrono>

namespace CPU{
	struct Reg{
	    char16_t SP, PC;
	    unsigned char A, F, B, C, D, E, H, L;
		bool IME;
	};


	extern Reg Registers;
	extern bool emuHalt;
	extern bool cpuHalt;
	extern bool step;
	extern bool continueExec;
	extern bool reload;
	extern bool shouldEnableInts;
	extern unsigned int freq;
	extern double timerFreq;
	void start();
	bool cycle();
	void clearState();
	void HandleInterrupt(std::string, unsigned char);
	char16_t readHL();
	char16_t readBC();
	char16_t readDE();
	char16_t readAF();
	extern inline void writeHL(char16_t);
	extern inline void writeBC(char16_t);
	extern inline void writeAF(char16_t);
	extern inline void writeDE(char16_t);
	extern inline void copyToC(int);
	extern inline void DAA();
	extern inline void EI();
	extern inline void DI();
}  // namespace CPU
