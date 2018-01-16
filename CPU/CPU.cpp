#include <vector>
#include <iostream>
#include "CPU.hpp"
#include "../Math/Math.hpp"
#include "../Memory/RAM.hpp"

//  Load / store instructions
#define LD_R_R(a, b) CPU::Registers.a = CPU::Registers.b
#define LD_R_D(a, b) CPU::Registers.a = b
#define LD_D_R(a, b) RAM::write(a, CPU::Registers.b)

//  Arithmetic
#define ADD_R_R(a, b) CPU::Registers.a += CPU::Registers.b
#define ADD_R_D(a, b) CPU::Registers.a += b
#define INC_R(a) CPU::Registers.a += 1
#define DEC_R(a) CPU::Registers.a -= 1
#define SUB_R_R(a,b) CPU::Registers.a -= CPU::Registers.b

//  Bitwise operations
#define AND_R(a) CPU::Registers.A &= CPU::Registers.a
#define AND_D(a) CPU::Registers.A &= a
#define OR_R(a) CPU::Registers.A |= CPU::Registers.a
#define OR_D(a) CPU::Registers.A |= a
#define XOR_R(a) CPU::Registers.A ^= CPU::Registers.a
#define XOR_D(a) CPU::Registers.A ^= a

//  For convenience, value in memory pointed to by register XX
#define BC_p RAM::read(CPU::readBC())
#define DE_p RAM::read(CPU::readDE())
#define HL_p RAM::read(CPU::readHL())

#define INSTRUCTIONS(x) \
x(0x00,1,4,                ) \
x(0x01,3,12,writeBC(d16)   ) \
x(0x02,1,8,                ) \
x(0x03,1,8,                ) \
x(0x04,1,4,                ) \
x(0x05,1,4,                ) \
x(0x06,1,4,                ) \
x(0x07,2,8,                ) \
x(0x08,3,20,               ) \
x(0x09,1,8,                ) \
x(0x0A,1,8,                ) \
x(0x0B,1,8,                ) \
x(0x0C,1,4,                ) \
x(0x0D,1,4,                ) \
x(0x0E,2,8,                ) \
x(0x0F,1,4,                ) \
x(0x10,2,4,writeDE(d16)	   ) \
x(0x11,3,12,               ) \
x(0x12,1,8,                ) \
x(0x13,1,8,                ) \
x(0x14,1,4,                ) \
x(0x15,1,4,                ) \
x(0x16,2,8,                ) \
x(0x17,1,4,                ) \
x(0x18,2,12,               ) \
x(0x19,1,8,                ) \
x(0x1A,1,8,                ) \
x(0x1B,1,8,                ) \
x(0x1C,1,4,                ) \
x(0x1D,1,4,                ) \
x(0x1E,2,8,                ) \
x(0x1F,1,4,                ) \
x(0x20,2,8,writeHL(d16)	   ) \
x(0x21,3,12,               ) \
x(0x22,1,8,                ) \
x(0x23,1,8,                ) \
x(0x24,1,4,                ) \
x(0x25,1,4,                ) \
x(0x26,2,8,                ) \
x(0x27,1,4,                ) \
x(0x28,2,8,                ) \
x(0x29,1,8,                ) \
x(0x2A,1,8,                ) \
x(0x2B,1,8,                ) \
x(0x2C,1,4,                ) \
x(0x2D,1,4,                ) \
x(0x2E,2,8,                ) \
x(0x2F,1,4,                ) \
x(0x30,2,8,LD_R_D(SP, d16) ) \
x(0x31,3,12,               ) \
x(0x32,1,8,                ) \
x(0x33,1,8,                ) \
x(0x34,1,12,               ) \
x(0x35,1,12,               ) \
x(0x36,2,12,               ) \
x(0x37,1,4,                ) \
x(0x38,2,8,                ) \
x(0x39,1,8,                ) \
x(0x3A,1,8,                ) \
x(0x3B,1,8,                ) \
x(0x3C,1,4,                ) \
x(0x3D,1,4,                ) \
x(0x3E,2,8,                ) \
x(0x3F,1,4,                ) \
x(0x40,1,4,LD_R_R(B, B)    ) \
x(0x41,1,4,LD_R_R(B, C)    ) \
x(0x42,1,4,LD_R_R(B, D)    ) \
x(0x43,1,4,LD_R_R(B, E)    ) \
x(0x44,1,4,LD_R_R(B, H)    ) \
x(0x45,1,4,LD_R_R(B, L)    ) \
x(0x46,1,8,LD_R_D(B, HL_p) ) \
x(0x47,1,4,LD_R_R(B, A)    ) \
x(0x48,1,4,LD_R_R(C, B)    ) \
x(0x49,1,4,LD_R_R(C, C)    ) \
x(0x4A,1,4,LD_R_R(C, D)    ) \
x(0x4B,1,4,LD_R_R(C, E)    ) \
x(0x4C,1,4,LD_R_R(C, H)    ) \
x(0x4D,1,4,LD_R_R(C, L)    ) \
x(0x4E,1,8,LD_R_D(C, HL_p) ) \
x(0x4F,1,4,LD_R_R(C, A)    ) \
x(0x50,1,4,LD_R_R(D, B)    ) \
x(0x51,1,4,LD_R_R(D, C)    ) \
x(0x52,1,4,LD_R_R(D, D)    ) \
x(0x53,1,4,LD_R_R(D, E)    ) \
x(0x54,1,4,LD_R_R(D, H)    ) \
x(0x55,1,4,LD_R_R(D, L)    ) \
x(0x56,1,8,LD_R_D(D, HL_p) ) \
x(0x57,1,4,LD_R_R(D, A)    ) \
x(0x58,1,4,LD_R_R(E, B)    ) \
x(0x59,1,4,LD_R_R(E, C)    ) \
x(0x5A,1,4,LD_R_R(E, D)    ) \
x(0x5B,1,4,LD_R_R(E, E)    ) \
x(0x5C,1,4,LD_R_R(E, H)    ) \
x(0x5D,1,4,LD_R_R(E, L)    ) \
x(0x5E,1,8,LD_R_D(E, HL_p) ) \
x(0x5F,1,4,LD_R_R(E, A)    ) \
x(0x60,1,4,LD_R_R(H, B)    ) \
x(0x61,1,4,LD_R_R(H, C)    ) \
x(0x62,1,4,LD_R_R(H, D)    ) \
x(0x63,1,4,LD_R_R(H, E)    ) \
x(0x64,1,4,LD_R_R(H, H)    ) \
x(0x65,1,4,LD_R_R(H, L)    ) \
x(0x66,1,8,LD_R_D(H, HL_p) ) \
x(0x67,1,4,LD_R_R(H, A)    ) \
x(0x68,1,4,LD_R_R(L, B)    ) \
x(0x69,1,4,LD_R_R(L, C)    ) \
x(0x6A,1,4,LD_R_R(L, D)    ) \
x(0x6B,1,4,LD_R_R(L, E)    ) \
x(0x6C,1,4,LD_R_R(L, H)    ) \
x(0x6D,1,4,LD_R_R(L, L)    ) \
x(0x6E,1,8,LD_R_D(L, HL_p) ) \
x(0x6F,1,4,LD_R_R(L, A)    ) \
x(0x70,1,8,LD_D_R(HL_p, B) ) \
x(0x71,1,8,LD_D_R(HL_p, C) ) \
x(0x72,1,8,LD_D_R(HL_p, D) ) \
x(0x73,1,8,LD_D_R(HL_p, E) ) \
x(0x74,1,8,LD_D_R(HL_p, H) ) \
x(0x75,1,8,LD_D_R(HL_p, L) ) \
x(0x76,1,4,CPU::halt = true) \
x(0x77,1,8,LD_D_R(HL_p, A) ) \
x(0x78,1,4,LD_R_R(A, B)    ) \
x(0x79,1,4,LD_R_R(A, C)    ) \
x(0x7A,1,4,LD_R_R(A, D)    ) \
x(0x7B,1,4,LD_R_R(A, E)    ) \
x(0x7C,1,4,LD_R_R(A, H)    ) \
x(0x7D,1,4,LD_R_R(A, L)    ) \
x(0x7E,1,8,LD_R_D(A, HL_p) ) \
x(0x7F,1,4,LD_R_R(A, A)    ) \
x(0x80,1,4,ADD_R_R(A, B)   ) \
x(0x81,1,4,ADD_R_R(A, C)   ) \
x(0x82,1,4,ADD_R_R(A, D)   ) \
x(0x83,1,4,ADD_R_R(A, E)   ) \
x(0x84,1,4,ADD_R_R(A, H)   ) \
x(0x85,1,4,ADD_R_R(A, L)   ) \
x(0x86,1,8,ADD_R_D(A, HL_p)) \
x(0x87,1,4,ADD_R_R(A, A)   ) \
x(0x88,1,4,                ) \
x(0x89,1,4,                ) \
x(0x8A,1,4,                ) \
x(0x8B,1,4,                ) \
x(0x8C,1,4,                ) \
x(0x8D,1,4,                ) \
x(0x8E,1,8,                ) \
x(0x8F,1,4,                ) \
x(0x90,1,4,SUB_R_R(A, B)   ) \
x(0x91,1,4,SUB_R_R(A, C)   ) \
x(0x92,1,4,SUB_R_R(A, D)   ) \
x(0x93,1,4,SUB_R_R(A, E)   ) \
x(0x94,1,4,SUB_R_R(A, H)   ) \
x(0x95,1,4,SUB_R_R(A, L)   ) \
x(0x96,1,8,SUB_R_D(A, HL_p)    ) \
x(0x97,1,4,SUB_R_R(A, A)    ) \
x(0x98,1,4,                ) \
x(0x99,1,4,                ) \
x(0x9A,1,4,                ) \
x(0x9B,1,4,                ) \
x(0x9C,1,4,                ) \
x(0x9D,1,4,                ) \
x(0x9E,1,8,                ) \
x(0x9F,1,4,                ) \
x(0xA0,1,4,AND_R(B)        ) \
x(0xA1,1,4,AND_R(C)        ) \
x(0xA2,1,4,AND_R(D)        ) \
x(0xA3,1,4,AND_R(E)        ) \
x(0xA4,1,4,AND_R(H)        ) \
x(0xA5,1,4,AND_R(L)        ) \
x(0xA6,1,8,AND_D(HL_p)     ) \
x(0xA7,1,4,AND_R(A)        ) \
x(0xA8,1,4,XOR_R(B)        ) \
x(0xA9,1,4,XOR_R(C)        ) \
x(0xAA,1,4,XOR_R(D)        ) \
x(0xAB,1,4,XOR_R(E)        ) \
x(0xAC,1,4,XOR_R(H)        ) \
x(0xAD,1,4,XOR_R(L)        ) \
x(0xAE,1,8,XOR_D(HL_p)     ) \
x(0xAF,1,4,XOR_R(A)        ) \
x(0xB0,1,4,OR_R(B)        ) \
x(0xB1,1,4,OR_R(C)        ) \
x(0xB2,1,4,OR_R(D)        ) \
x(0xB3,1,4,OR_R(E)        ) \
x(0xB4,1,4,OR_R(H)        ) \
x(0xB5,1,4,OR_R(L)        ) \
x(0xB6,1,8,OR_D(HL_p)     ) \
x(0xB7,1,4,OR_R(A)        ) \
x(0xB8,1,4,                ) \
x(0xB9,1,4,                ) \
x(0xBA,1,4,                ) \
x(0xBB,1,4,                ) \
x(0xBC,1,4,                ) \
x(0xBD,1,4,                ) \
x(0xBE,1,8,                ) \
x(0xBF,1,4,                ) \
x(0xC0,1,8,                ) \
x(0xC1,1,12,               ) \
x(0xC2,3,12,               ) \
x(0xC3,3,16,               ) \
x(0xC4,3,12,               ) \
x(0xC5,1,16,               ) \
x(0xC6,2,8,                ) \
x(0xC7,1,16,               ) \
x(0xC8,1,8,                ) \
x(0xC9,1,16,               ) \
x(0xCA,3,12,               ) \
x(0xCB,1,4,                ) \
x(0xCC,3,12,               ) \
x(0xCD,3,24,               ) \
x(0xCE,2,8,                ) \
x(0xCF,1,16,               ) \
x(0xD0,1,8,                ) \
x(0xD1,1,12,               ) \
x(0xD2,3,12,               ) \
x(0xD4,3,12,               ) \
x(0xD5,1,16,               ) \
x(0xD6,2,8,                ) \
x(0xD7,1,16,               ) \
x(0xD8,1,8,                ) \
x(0xD9,1,16,               ) \
x(0xDA,3,12,               ) \
x(0xDC,3,12,               ) \
x(0xDE,2,8,                ) \
x(0xDF,1,16,               ) \
x(0xE0,2,12,               ) \
x(0xE1,1,12,               ) \
x(0xE2,2,8,                ) \
x(0xE5,1,16,               ) \
x(0xE6,2,8,                ) \
x(0xE7,1,16,               ) \
x(0xE8,2,16,               ) \
x(0xE9,1,4,                ) \
x(0xEA,3,16,               ) \
x(0xEE,2,8,                ) \
x(0xEF,1,16,               ) \
x(0xF0,2,12,               ) \
x(0xF1,1,12,               ) \
x(0xF2,2,8,                ) \
x(0xF3,1,4,                ) \
x(0xF5,1,16,               ) \
x(0xF6,2,8,                ) \
x(0xF7,1,16,               ) \
x(0xF8,2,12,               ) \
x(0xF9,1,8,                ) \
x(0xFA,3,16,               ) \
x(0xFB,1,4,                ) \
x(0xFE,2,8,                ) \
x(0xFF,1,16,               )


namespace CPU{
	Reg Registers;
	bool halt = false;
	std::vector<char16_t> Stack;
	int64_t cycles;

	/*
		Starts the emulator CPU
	*/
	void start(){
		while(!halt){
			cycle();
		}
	}

	/*
		Executes one CPU cycle/instruction
	*/
	bool cycle(){
		unsigned char prefix = RAM::read(Registers.PC),
					  opcode = RAM::read(Registers.PC + 0x001);
		if(prefix != 0xCB){
			//  Use normal op table
			char16_t d16 = RAM::read(Registers.PC + 0x001)*0x100 + RAM::read(Registers.PC + 0x002),
				   	 a16 = d16;
			char     d8 = RAM::read(Registers.PC + 0x001),
					 r8 = d8;
			unsigned char a8 = RAM::read(Registers.PC + 0x001);

			switch(prefix){
				#define x(op,byte,cycle,ins) case op: ins; cycles += cycle; Registers.PC += byte; break;
				INSTRUCTIONS(x)
				#undef x

				default: std::cout << "UNKNOWN OPCODE: " << Math::decHex(prefix) << "\n";
			}
		} else {
			// Use CB table
		}


		return true;
	}

	/*
		Clears the state of the CPU
		Eventually allow for randomization of initial memory
	*/
	void clearState(){
		Registers.SP = Registers.PC = Registers.A = Registers.B \
					 = Registers.C = Registers.D = Registers.E \
					 = Registers.H = Registers.L = 0x0;
	}


	/*
		Read and write to the special registers
		For convenience
	*/
	inline char16_t readHL(){
		return Registers.H * 0x100 + Registers.L;
	}

	inline char16_t readBC(){
		return Registers.B * 0x100 + Registers.C;
	}

	inline char16_t readDE(){
		return Registers.D * 0x100 + Registers.E;
	}

	inline void writeHL(char16_t data){
		unsigned char H = data / 0x100,
					  L = data - H*0x100;
		Registers.H = H;
		Registers.L = L;
	}

	inline void writeBC(char16_t data){
		unsigned char B = data / 0x100,
					  C = data - B*0x100;
		Registers.B = B;
		Registers.C = C;
}

	inline void writeDE(char16_t data){
		unsigned char D = data / 0x100,
					  E = data - D*0x100;
		Registers.D = D;
		Registers.E = E;
	}
}  // namespace CPU

