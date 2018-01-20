#include <vector>
#include <iostream>
#include "CPU.hpp"
#include "../Math/Math.hpp"
#include "../Memory/RAM.hpp"

// Flag reading
#define readZ	((Registers.F >> 7))
#define readN   ((Registers.F & 0x40) >> 6)
#define readH   ((Registers.F & 0x20) >> 5)
#define readC   ((Registers.F & 0x10) >> 4)

#define setZ  (Registers.F |= 0x80)
#define setN  (Registers.F |= 0x40)
#define setH  (Registers.F |= 0x20)
#define setC  (Registers.F |= 0x10)

#define unsetZ  (Registers.F &= ~0x80)
#define unsetN  (Registers.F &= ~0x40)
#define unsetH  (Registers.F &= ~0x20)
#define unsetC  (Registers.F &= ~0x10)

//  Modifying bits
#define setBitN_R(a, b)  (Registers.b |= (0x01 << a))
#define setBitN_D(a, b)  (RAM::write(b, RAM::read(b) | (0x01 << a)))
#define resetBitN_R(a, b)  (Registers.b &= ~(1 << a))
#define resetBitN_D(a, b)  (RAM::write(b, b & ~(1 << a)))
#define SWAP_R(a) 	(Registers.a = (Registers.a >> 4) | (Registers.a << 4)); if(Registers.a == 0){ setZ; }
#define SWAP_D(a)	RAM::write(a, (RAM::read(a) >> 4) | (RAM::read(a) << 4)); if(RAM::read(a) == 0){ setZ; }

#define SLA_R(a)	copyToC(Registers.a >> 7); \
					Registers.a = Registers.a << 1; \
					Registers.a &= ~(1); \
					if(Registers.a == 0){ setZ; }
					
#define SLA_D(a)	copyToC(RAM::read(a) >> 7); \
					RAM::write(a, RAM::read(a) << 1); \
					RAM::write(a, RAM::read(a) & ~(1)); \
					if(RAM::read(a) == 0){ setZ; }

#define SRA_R(a)	copyToC(Registers.a & 1); \
					Registers.a = (Registers.a >> 1) | (Registers.a & 0x80); \
					if(Registers.a == 0){ setZ; }

#define SRA_D(a)	copyToC(RAM::read(a) & 1); \
					RAM::write(a, (RAM::read(a) >> 1) | (RAM::read(a) & 0x80)); \
					if(RAM::read(a) == 0){ setZ; }

#define RL_R(a)		int temp = readC, bit7 = Registers.a >> 7; \
					copyToC(bit7); \
					Registers.a = Registers.a << 1; \
					Registers.a = ((Registers.a & (~1) ) | temp); \
					if(Registers.a == 0){ setZ; }

#define RR_R(a)		int temp = readC, bit0 = Registers.a & 1; \
					copyToC(bit0); \
					Registers.a = Registers.a >> 1; \
					Registers.a = (Registers.a & (~(1 << 7))) | (temp << 7); \
					if(Registers.a == 0){ setZ; }

#define RL_D(a)		int temp = readC, bit7 = RAM::read(a) >> 7; \
					copyToC(bit7); \
					RAM::write(a, RAM::read(a) << 1 ); \
					RAM::write(a, ( RAM::read(a) & (~1) )| temp); \
					if(RAM::read(a) == 0){ setZ; }

#define RR_D(a)		int temp = readC, bit0 = RAM::read(a) & 1; \
					copyToC(bit0); \
					RAM::write(a, RAM::read(a) >> 1); \
					RAM::write(a, ( RAM::read(a) & (~(1 << 7)) ) | (temp << 7)); \
					if(RAM::read(a) == 0){ setZ; }

#define RLC_R(a)	int bit7 = Registers.a >> 7; \
					copyToC(bit7); \
					Registers.a = Registers.a << 1; \
					Registers.a = (Registers.a & (~1)) | bit7; \
					if(Registers.a == 0){ setZ; }

#define RRC_R(a)	int bit0 = Registers.a & 1; \
					copyToC(bit0); \
					Registers.a = Registers.a >> 1; \
					Registers.a = (Registers.a & ~(1 << 7)) | (bit0 << 7); \
					if(Registers.a == 0){ setZ; }

#define RLC_D(a)	int bit7 = RAM::read(a) >> 7; \
					copyToC(bit7); \
					RAM::write(a, RAM::read(a) << 1); \
					RAM::write(a, (RAM::read(a) & (~1)) | bit7); \
					if(RAM::read(a) == 0){ setZ; }

#define RRC_D(a)	int bit0 = RAM::read(a) & 1; \
					copyToC(bit0); \
					RAM::write(a, RAM::read(a) >> 1); \
					RAM::write(a, (RAM::read(a) & ~(1 << 7)) | (bit0 << 7)); \
					if(RAM::read(a) == 0){ setZ; }

//  Bit testing
#define BIT_X_R(a,b) (Registers.b & (1 << a))
#define BIT_X_D(a,b) (b & (1 << a))

#define BITN_R(a,b)  if(BIT_X_R(a,b)){ setZ; } else { unsetZ; }
#define BITN_D(a,b)  if(BIT_X_D(a,b)){ setZ; } else { unsetZ; }

//  Load / store instructions
#define LD_R_R(a, b) CPU::Registers.a = CPU::Registers.b
#define LD_R_D(a, b) CPU::Registers.a = b
#define LD_D_R(a, b) RAM::write(a, CPU::Registers.b)
#define LDI_D_R(a, b) RAM::write(a, CPU::Registers.b); writeHL(readHL() + 1)
#define LDI_R_D(a, b) CPU::Registers.a = RAM::read(b); writeHL(readHL() + 1)
#define LDD_D_R(a, b) RAM::write(a, CPU::Registers.b); writeHL(readHL() - 1)
#define LDD_R_D(a, b) CPU::Registers.a = RAM::read(b); writeHL(readHL() - 1)
#define JR_CC_N(a, b) if(a){ CPU::Registers.PC += b; }
#define JR_N(a)	CPU::Registers.PC += a

//  Arithmetic
#define ADD_R_R(a, b) CPU::Registers.a += CPU::Registers.b
#define ADD_R_D(a, b) CPU::Registers.a += b
#define ADC_R(a) CPU::Registers.A = CPU::Registers.A + CPU::Registers.a + readC
#define ADC_D(a) CPU::Registers.A = CPU::Registers.A + a + readC
#define INC_R(a) CPU::Registers.a += 1
#define INC_D(a) RAM::write(a, RAM::read(a) + 0x01)
#define DEC_R(a) setN; CPU::Registers.a -= 1
#define DEC_D(a) RAM::write(a, RAM::read(a) - 0x01)
#define SUB_R_R(a,b) CPU::Registers.a -= CPU::Registers.b
#define SUB_R_D(a,b) CPU::Registers.a -= b
#define SBC_R(a) CPU::Registers.A = CPU::Registers.A - CPU::Registers.a - readC
#define SBC_D(a) CPU::Registers.A = CPU::Registers.A - a - readC

#define CP_R(a)	if(CPU::Registers.A - CPU::Registers.a == 0){ setZ; } \
				else if(CPU::Registers.A < CPU::Registers.a){ setC; } 
#define CP_D(a) if(CPU::Registers.A - a == 0){ setZ; } \
				else if(CPU::Registers.A < a){ setC; } 
//																													!!!! IMPORTANT: DO HALF CARRY FLAG



//  Bitwise operations
#define AND_R(a) CPU::Registers.A &= CPU::Registers.a
#define AND_D(a) CPU::Registers.A &= a
#define OR_R(a) CPU::Registers.A |= CPU::Registers.a
#define OR_D(a) CPU::Registers.A |= a
#define XOR_R(a) CPU::Registers.A ^= CPU::Registers.a
#define XOR_D(a) CPU::Registers.A ^= a

//  For convenience, value in memory pointed to by register XX
//  OH GOD WHY DOES THIS HAPPEN TO ME
//  THIS IS PROBABLY VERY WRONG
#define BC_p RAM::read(CPU::readBC())
#define DE_p RAM::read(CPU::readDE())
#define HL_p RAM::read(CPU::readHL())
#define C_p  RAM::read(CPU::Registers.C)

#define INSTRUCTIONS(x) \
x(0x0, 0x0, 0x00,1,4, /* filler */  ) \
x(0x0, 0x0, 0x01,3,12,writeBC(d16)   ) \
x(0x0, 0x0, 0x02,1,8,LD_D_R(BC_p, A) ) \
x(0x0, 0x0, 0x03,1,8,writeBC(readBC() + 0x01)) \
x(0x0, 0x0, 0x04,1,4,INC_R(B)		   ) \
x(0x0, 0x0, 0x05,1,4,DEC_R(B)        ) \
x(0x0, 0x0, 0x06,1,4,LD_R_D(B, d8)   ) \
x(0x6, 0x0, 0x07,2,8,RLC_R(A) 		) \
x(0x0, 0x0, 0x08,3,20,LD_D_R(a16, SP)) \
x(0x0, 0x0, 0x09,1,8,writeHL(readHL() + readBC())) \
x(0x0, 0x0, 0x0A,1,8,LD_R_D(A, BC_p) ) \
x(0x0, 0x0, 0x0B,1,8,writeBC(readBC() - 0x01)) \
x(0x0, 0x0, 0x0C,1,4,INC_R(C)        ) \
x(0x0, 0x0, 0x0D,1,4,DEC_R(C)		   ) \
x(0x0, 0x0, 0x0E,2,8,LD_R_D(C, d8)   ) \
x(0x6, 0x0, 0x0F,1,4,RRC_R(A)			   ) \
x(0x0, 0x0, 0x10,2,4,				   ) \
x(0x0, 0x0, 0x11,3,12,writeDE(d16)   ) \
x(0x0, 0x0, 0x12,1,8,LD_D_R(DE_p, A) ) \
x(0x0, 0x0, 0x13,1,8,writeDE(readDE() + 0x01)                ) \
x(0x0, 0x0, 0x14,1,4,INC_R(D)        ) \
x(0x0, 0x0, 0x15,1,4,DEC_R(B)        ) \
x(0x0, 0x0, 0x16,2,8,LD_R_D(D, d8)   ) \
x(0x6, 0x0, 0x17,1,4,RL_R(A) 			   ) \
x(0x0, 0x0, 0x18,2,12,JR_N(r8) ) \
x(0x0, 0x0, 0x19,1,8,writeHL(readHL() + readDE())) \
x(0x0, 0x0, 0x1A,1,8,LD_R_D(A, DE_p) ) \
x(0x0, 0x0, 0x1B,1,8,writeDE(readDE() - 0x01)) \
x(0x0, 0x0, 0x1C,1,4,INC_R(E)        ) \
x(0x0, 0x0, 0x1D,1,4,DEC_R(E)        ) \
x(0x0, 0x0, 0x1E,2,8,LD_R_D(E, d8)   ) \
x(0x6, 0x0, 0x1F,1,4,RR_R(A)                ) \
x(0x0, 0x0, 0x20,2,8,JR_CC_N(!readZ, r8) ) \
x(0x0, 0x0, 0x21,3,12,writeHL(d16)   ) \
x(0x0, 0x0, 0x22,1,8,LDI_D_R(HL_p, A) ) \
x(0x0, 0x0, 0x23,1,8,writeHL(readHL() + 1)                ) \
x(0x0, 0x0, 0x24,1,4,INC_R(H)        ) \
x(0x0, 0x0, 0x25,1,4,DEC_R(H)        ) \
x(0x0, 0x0, 0x26,2,8,LD_R_D(H, d8)   ) \
x(0x0, 0x0, 0x27,1,4,                ) \
x(0x0, 0x0, 0x28,2,8,JR_CC_N(readZ, r8)                ) \
x(0x0, 0x0, 0x29,1,8,writeHL(2*readHL())                ) \
x(0x0, 0x0, 0x2A,1,8,LDI_R_D(A, HL_p)            ) \
x(0x0, 0x0, 0x2B,1,8,writeHL(readHL() - 0x01 )                ) \
x(0x0, 0x0, 0x2C,1,4,INC_R(L)		   ) \
x(0x0, 0x0, 0x2D,1,4,DEC_R(L)        ) \
x(0x0, 0x0, 0x2E,2,8,LD_R_D(L, d8)   ) \
x(0x0, 0x6, 0x2F,1,4, (CPU::Registers.A = ~CPU::Registers.A)    ) \
x(0x0, 0x0, 0x30,2,8,JR_CC_N(!readC, r8) 			   ) \
x(0x0, 0x0, 0x31,3,12,LD_R_D(SP, d16)) \
x(0x0, 0x0, 0x32,1,8,LDD_D_R(HL_p, A)           ) \
x(0x0, 0x0, 0x33,1,8,INC_R(SP)	   ) \
x(0x0, 0x0, 0x34,1,12,writeHL(readHL() + 0x01) ) \
x(0x0, 0x0, 0x35,1,12,writeHL(readHL() - 0x01) ) \
x(0x0, 0x0, 0x36,2,12,writeHL(d8)	   ) \
x(0x6, 0x1, 0x37,1,4,setC           ) \
x(0x0, 0x0, 0x38,2,8,JR_CC_N(readC, r8)  ) \
x(0x0, 0x0, 0x39,1,8,writeHL(readHL() + Registers.SP)) \
x(0x0, 0x0, 0x3A,1,8,LDD_R_D(A, HL_p)               ) \
x(0x0, 0x0, 0x3B,1,8,DEC_R(SP)       ) \
x(0x0, 0x0, 0x3C,1,4,INC_R(A)        ) \
x(0x0, 0x0, 0x3D,1,4,DEC_R(A)		   ) \
x(0x0, 0x0, 0x3E,2,8,LD_R_D(A, d8)   ) \
x(0x4, 0x0, 0x3F,1,4,Registers.F ^= 0x10 ) \
x(0x0, 0x0, 0x40,1,4,LD_R_R(B, B)    ) \
x(0x0, 0x0, 0x41,1,4,LD_R_R(B, C)    ) \
x(0x0, 0x0, 0x42,1,4,LD_R_R(B, D)    ) \
x(0x0, 0x0, 0x43,1,4,LD_R_R(B, E)    ) \
x(0x0, 0x0, 0x44,1,4,LD_R_R(B, H)    ) \
x(0x0, 0x0, 0x45,1,4,LD_R_R(B, L)    ) \
x(0x0, 0x0, 0x46,1,8,LD_R_D(B, HL_p) ) \
x(0x0, 0x0, 0x47,1,4,LD_R_R(B, A)    ) \
x(0x0, 0x0, 0x48,1,4,LD_R_R(C, B)    ) \
x(0x0, 0x0, 0x49,1,4,LD_R_R(C, C)    ) \
x(0x0, 0x0, 0x4A,1,4,LD_R_R(C, D)    ) \
x(0x0, 0x0, 0x4B,1,4,LD_R_R(C, E)    ) \
x(0x0, 0x0, 0x4C,1,4,LD_R_R(C, H)    ) \
x(0x0, 0x0, 0x4D,1,4,LD_R_R(C, L)    ) \
x(0x0, 0x0, 0x4E,1,8,LD_R_D(C, HL_p) ) \
x(0x0, 0x0, 0x4F,1,4,LD_R_R(C, A)    ) \
x(0x0, 0x0, 0x50,1,4,LD_R_R(D, B)    ) \
x(0x0, 0x0, 0x51,1,4,LD_R_R(D, C)    ) \
x(0x0, 0x0, 0x52,1,4,LD_R_R(D, D)    ) \
x(0x0, 0x0, 0x53,1,4,LD_R_R(D, E)    ) \
x(0x0, 0x0, 0x54,1,4,LD_R_R(D, H)    ) \
x(0x0, 0x0, 0x55,1,4,LD_R_R(D, L)    ) \
x(0x0, 0x0, 0x56,1,8,LD_R_D(D, HL_p) ) \
x(0x0, 0x0, 0x57,1,4,LD_R_R(D, A)    ) \
x(0x0, 0x0, 0x58,1,4,LD_R_R(E, B)    ) \
x(0x0, 0x0, 0x59,1,4,LD_R_R(E, C)    ) \
x(0x0, 0x0, 0x5A,1,4,LD_R_R(E, D)    ) \
x(0x0, 0x0, 0x5B,1,4,LD_R_R(E, E)    ) \
x(0x0, 0x0, 0x5C,1,4,LD_R_R(E, H)    ) \
x(0x0, 0x0, 0x5D,1,4,LD_R_R(E, L)    ) \
x(0x0, 0x0, 0x5E,1,8,LD_R_D(E, HL_p) ) \
x(0x0, 0x0, 0x5F,1,4,LD_R_R(E, A)    ) \
x(0x0, 0x0, 0x60,1,4,LD_R_R(H, B)    ) \
x(0x0, 0x0, 0x61,1,4,LD_R_R(H, C)    ) \
x(0x0, 0x0, 0x62,1,4,LD_R_R(H, D)    ) \
x(0x0, 0x0, 0x63,1,4,LD_R_R(H, E)    ) \
x(0x0, 0x0, 0x64,1,4,LD_R_R(H, H)    ) \
x(0x0, 0x0, 0x65,1,4,LD_R_R(H, L)    ) \
x(0x0, 0x0, 0x66,1,8,LD_R_D(H, HL_p) ) \
x(0x0, 0x0, 0x67,1,4,LD_R_R(H, A)    ) \
x(0x0, 0x0, 0x68,1,4,LD_R_R(L, B)    ) \
x(0x0, 0x0, 0x69,1,4,LD_R_R(L, C)    ) \
x(0x0, 0x0, 0x6A,1,4,LD_R_R(L, D)    ) \
x(0x0, 0x0, 0x6B,1,4,LD_R_R(L, E)    ) \
x(0x0, 0x0, 0x6C,1,4,LD_R_R(L, H)    ) \
x(0x0, 0x0, 0x6D,1,4,LD_R_R(L, L)    ) \
x(0x0, 0x0, 0x6E,1,8,LD_R_D(L, HL_p) ) \
x(0x0, 0x0, 0x6F,1,4,LD_R_R(L, A)    ) \
x(0x0, 0x0, 0x70,1,8,LD_D_R(HL_p, B) ) \
x(0x0, 0x0, 0x71,1,8,LD_D_R(HL_p, C) ) \
x(0x0, 0x0, 0x72,1,8,LD_D_R(HL_p, D) ) \
x(0x0, 0x0, 0x73,1,8,LD_D_R(HL_p, E) ) \
x(0x0, 0x0, 0x74,1,8,LD_D_R(HL_p, H) ) \
x(0x0, 0x0, 0x75,1,8,LD_D_R(HL_p, L) ) \
x(0x0, 0x0, 0x76,1,4,CPU::halt = true) \
x(0x0, 0x0, 0x77,1,8,LD_D_R(HL_p, A) ) \
x(0x0, 0x0, 0x78,1,4,LD_R_R(A, B)    ) \
x(0x0, 0x0, 0x79,1,4,LD_R_R(A, C)    ) \
x(0x0, 0x0, 0x7A,1,4,LD_R_R(A, D)    ) \
x(0x0, 0x0, 0x7B,1,4,LD_R_R(A, E)    ) \
x(0x0, 0x0, 0x7C,1,4,LD_R_R(A, H)    ) \
x(0x0, 0x0, 0x7D,1,4,LD_R_R(A, L)    ) \
x(0x0, 0x0, 0x7E,1,8,LD_R_D(A, HL_p) ) \
x(0x0, 0x0, 0x7F,1,4,LD_R_R(A, A)    ) \
x(0x0, 0x0, 0x80,1,4,ADD_R_R(A, B)   ) \
x(0x0, 0x0, 0x81,1,4,ADD_R_R(A, C)   ) \
x(0x0, 0x0, 0x82,1,4,ADD_R_R(A, D)   ) \
x(0x0, 0x0, 0x83,1,4,ADD_R_R(A, E)   ) \
x(0x0, 0x0, 0x84,1,4,ADD_R_R(A, H)   ) \
x(0x0, 0x0, 0x85,1,4,ADD_R_R(A, L)   ) \
x(0x0, 0x0, 0x86,1,8,ADD_R_D(A, HL_p)) \
x(0x0, 0x0, 0x87,1,4,ADD_R_R(A, A)   ) \
x(0x0, 0x0, 0x88,1,4,ADC_R(B)        ) \
x(0x0, 0x0, 0x89,1,4,ADC_R(C)		   ) \
x(0x0, 0x0, 0x8A,1,4,ADC_R(D)		   ) \
x(0x0, 0x0, 0x8B,1,4,ADC_R(E)		   ) \
x(0x0, 0x0, 0x8C,1,4,ADC_R(H)		   ) \
x(0x0, 0x0, 0x8D,1,4,ADC_R(L)		   ) \
x(0x0, 0x0, 0x8E,1,8,ADC_D(HL_p)     ) \
x(0x0, 0x0, 0x8F,1,4,ADC_R(A)        ) \
x(0x0, 0x0, 0x90,1,4,SUB_R_R(A, B)   ) \
x(0x0, 0x0, 0x91,1,4,SUB_R_R(A, C)   ) \
x(0x0, 0x0, 0x92,1,4,SUB_R_R(A, D)   ) \
x(0x0, 0x0, 0x93,1,4,SUB_R_R(A, E)   ) \
x(0x0, 0x0, 0x94,1,4,SUB_R_R(A, H)   ) \
x(0x0, 0x0, 0x95,1,4,SUB_R_R(A, L)   ) \
x(0x0, 0x0, 0x96,1,8,SUB_R_D(A, HL_p)) \
x(0x0, 0x0, 0x97,1,4,SUB_R_R(A, A)   ) \
x(0x0, 0x0, 0x98,1,4,SBC_R(B)   	   ) \
x(0x0, 0x0, 0x99,1,4,SBC_R(C)		   ) \
x(0x0, 0x0, 0x9A,1,4,SBC_R(D)		   ) \
x(0x0, 0x0, 0x9B,1,4,SBC_R(E)		   ) \
x(0x0, 0x0, 0x9C,1,4,SBC_R(H)		   ) \
x(0x0, 0x0, 0x9D,1,4,SBC_R(L)		   ) \
x(0x0, 0x0, 0x9E,1,8,SBC_D(HL_p)	   ) \
x(0x0, 0x0, 0x9F,1,4,SBC_R(A)   	   ) \
x(0x5, 0x2, 0xA0,1,4,AND_R(B)        ) \
x(0x5, 0x2, 0xA1,1,4,AND_R(C)        ) \
x(0x5, 0x2, 0xA2,1,4,AND_R(D)        ) \
x(0x5, 0x2, 0xA3,1,4,AND_R(E)        ) \
x(0x5, 0x2, 0xA4,1,4,AND_R(H)        ) \
x(0x5, 0x2, 0xA5,1,4,AND_R(L)        ) \
x(0x5, 0x2, 0xA6,1,8,AND_D(HL_p)     ) \
x(0x5, 0x2, 0xA7,1,4,AND_R(A)        ) \
x(0x7, 0x0, 0xA8,1,4,XOR_R(B)        ) \
x(0x7, 0x0, 0xA9,1,4,XOR_R(C)        ) \
x(0x7, 0x0, 0xAA,1,4,XOR_R(D)        ) \
x(0x7, 0x0, 0xAB,1,4,XOR_R(E)        ) \
x(0x7, 0x0, 0xAC,1,4,XOR_R(H)        ) \
x(0x7, 0x0, 0xAD,1,4,XOR_R(L)        ) \
x(0x7, 0x0, 0xAE,1,8,XOR_D(HL_p)     ) \
x(0x7, 0x0, 0xAF,1,4,XOR_R(A)        ) \
x(0x7, 0x0, 0xB0,1,4,OR_R(B)        ) \
x(0x7, 0x0, 0xB1,1,4,OR_R(C)        ) \
x(0x7, 0x0, 0xB2,1,4,OR_R(D)        ) \
x(0x7, 0x0, 0xB3,1,4,OR_R(E)        ) \
x(0x7, 0x0, 0xB4,1,4,OR_R(H)        ) \
x(0x7, 0x0, 0xB5,1,4,OR_R(L)        ) \
x(0x7, 0x0, 0xB6,1,8,OR_D(HL_p)     ) \
x(0x7, 0x0, 0xB7,1,4,OR_R(A)        ) \
x(0x0, 0x0, 0xB8,1,4,CP_R(B)                ) \
x(0x0, 0x0, 0xB9,1,4,CP_R(C)                ) \
x(0x0, 0x0, 0xBA,1,4,CP_R(D)                ) \
x(0x0, 0x0, 0xBB,1,4,CP_R(E)                ) \
x(0x0, 0x0, 0xBC,1,4,CP_R(H)                ) \
x(0x0, 0x0, 0xBD,1,4,CP_R(L)                ) \
x(0x0, 0x0, 0xBE,1,8,CP_D(HL_p)                ) \
x(0x0, 0x0, 0xBF,1,4,CP_R(A)                ) \
x(0x0, 0x0, 0xC0,1,8,                ) \
x(0x0, 0x0, 0xC1,1,12,               ) \
x(0x0, 0x0, 0xC2,3,12,JR_CC_N(!readZ,a16)               ) \
x(0x0, 0x0, 0xC3,3,16,JR_N(a16)               ) \
x(0x0, 0x0, 0xC4,3,12,               ) \
x(0x0, 0x0, 0xC5,1,16,               ) \
x(0x0, 0x0, 0xC6,2,8,ADD_R_D(A, d8)  ) \
x(0x0, 0x0, 0xC7,1,16,               ) \
x(0x0, 0x0, 0xC8,1,8,                ) \
x(0x0, 0x0, 0xC9,1,16,               ) \
x(0x0, 0x0, 0xCA,3,12,JR_CC_N(readZ, a16)               ) \
x(0x0, 0x0, 0xCC,3,12,               ) \
x(0x0, 0x0, 0xCD,3,24,               ) \
x(0x0, 0x0, 0xCE,2,8,                ) \
x(0x0, 0x0, 0xCF,1,16,               ) \
x(0x0, 0x0, 0xD0,1,8,                ) \
x(0x0, 0x0, 0xD1,1,12,               ) \
x(0x0, 0x0, 0xD2,3,12,JR_CC_N(!readC,a16)               ) \
x(0x0, 0x0, 0xD4,3,12,               ) \
x(0x0, 0x0, 0xD5,1,16,               ) \
x(0x0, 0x0, 0xD6,2,8,                ) \
x(0x0, 0x0, 0xD7,1,16,               ) \
x(0x0, 0x0, 0xD8,1,8,                ) \
x(0x0, 0x0, 0xD9,1,16,               ) \
x(0x0, 0x0, 0xDA,3,12,JR_CC_N(readC, a16)               ) \
x(0x0, 0x0, 0xDC,3,12,               ) \
x(0x0, 0x0, 0xDE,2,8,                ) \
x(0x0, 0x0, 0xDF,1,16,               ) \
x(0x0, 0x0, 0xE0,2,12,               ) \
x(0x0, 0x0, 0xE1,1,12,               ) \
x(0x0, 0x0, 0xE2,2,8,LD_D_R(0xFF00 + CPU::Registers.C, A)    ) \
x(0x0, 0x0, 0xE5,1,16,               ) \
x(0x5, 0x2, 0xE6,2,8,                ) \
x(0x0, 0x0, 0xE7,1,16,               ) \
x(0x0, 0x0, 0xE8,2,16,ADD_R_D(SP, r8)) \
x(0x0, 0x0, 0xE9,1,4,                ) \
x(0x0, 0x0, 0xEA,3,16,               ) \
x(0x7, 0x0, 0xEE,2,8,                ) \
x(0x0, 0x0, 0xEF,1,16,               ) \
x(0x0, 0x0, 0xF0,2,12,               ) \
x(0x0, 0x0, 0xF1,1,12,               ) \
x(0x0, 0x0, 0xF2,2,8,LD_R_D(A, RAM::read(0xFF00 + CPU::Registers.C) ) ) \
x(0x0, 0x0, 0xF3,1,4,                ) \
x(0x0, 0x0, 0xF5,1,16,               ) \
x(0x7, 0x0, 0xF6,2,8,                ) \
x(0x0, 0x0, 0xF7,1,16,               ) \
x(0x0, 0x0, 0xF8,2,12,               ) \
x(0x0, 0x0, 0xF9,1,8,Registers.SP = readHL() ) \
x(0x0, 0x0, 0xFA,3,16,               ) \
x(0x0, 0x0, 0xFB,1,4,                ) \
x(0x0, 0x0, 0xFE,2,8,                ) \
x(0x0, 0x0, 0xFF,1,16,               )

//  End of INSTRUCTIONS()

#define INSTRUCTIONS_CB(y) \
y(0x6, 0x0, 0x00,2,8, RLC_R(B)               ) \
y(0x6, 0x0, 0x01,2,8, RLC_R(C)               ) \
y(0x6, 0x0, 0x02,2,8, RLC_R(D)               ) \
y(0x6, 0x0, 0x03,2,8, RLC_R(E)               ) \
y(0x6, 0x0, 0x04,2,8, RLC_R(H)               ) \
y(0x6, 0x0, 0x05,2,8, RLC_R(L)               ) \
y(0x6, 0x0, 0x06,2,15,RLC_D(HL_p)                ) \
y(0x6, 0x0, 0x07,2,8, RLC_R(A)               ) \
y(0x6, 0x0, 0x08,2,8, RRC_R(B)               ) \
y(0x6, 0x0, 0x09,2,8, RRC_R(C)               ) \
y(0x6, 0x0, 0x0A,2,8, RRC_R(D)               ) \
y(0x6, 0x0, 0x0B,2,8, RRC_R(E)               ) \
y(0x6, 0x0, 0x0C,2,8, RRC_R(H)               ) \
y(0x6, 0x0, 0x0D,2,8, RRC_R(L)               ) \
y(0x6, 0x0, 0x0E,2,15,RRC_D(HL_p)                ) \
y(0x6, 0x0, 0x0F,2,8, RRC_R(A)               ) \
y(0x6, 0x0, 0x10,2,8, RL_R(B)               ) \
y(0x6, 0x0, 0x11,2,8, RL_R(C)               ) \
y(0x6, 0x0, 0x12,2,8, RL_R(D)               ) \
y(0x6, 0x0, 0x13,2,8, RL_R(E)               ) \
y(0x6, 0x0, 0x14,2,8, RL_R(H)               ) \
y(0x6, 0x0, 0x15,2,8, RL_R(L)               ) \
y(0x6, 0x0, 0x16,2,15,RL_D(HL_p)                ) \
y(0x6, 0x0, 0x17,2,8, RL_R(A)               ) \
y(0x6, 0x0, 0x18,2,8, RR_R(B)               ) \
y(0x6, 0x0, 0x19,2,8, RR_R(C)               ) \
y(0x6, 0x0, 0x1A,2,8, RR_R(D)               ) \
y(0x6, 0x0, 0x1B,2,8, RR_R(E)               ) \
y(0x6, 0x0, 0x1C,2,8, RR_R(H)               ) \
y(0x6, 0x0, 0x1D,2,8, RR_R(L)               ) \
y(0x6, 0x0, 0x1E,2,15,RR_D(HL_p)                ) \
y(0x6, 0x0, 0x1F,2,8, RR_R(A)               ) \
y(0x6, 0x0, 0x20,2,8, SLA_R(B)               ) \
y(0x6, 0x0, 0x21,2,8, SLA_R(C)               ) \
y(0x6, 0x0, 0x22,2,8, SLA_R(D)               ) \
y(0x6, 0x0, 0x23,2,8, SLA_R(E)               ) \
y(0x6, 0x0, 0x24,2,8, SLA_R(H)               ) \
y(0x6, 0x0, 0x25,2,8, SLA_R(L)               ) \
y(0x6, 0x0, 0x26,2,15,SLA_D(HL_p)                ) \
y(0x6, 0x0, 0x27,2,8, SLA_R(A)               ) \
y(0x6, 0x0, 0x28,2,8, SRA_R(B)               ) \
y(0x6, 0x0, 0x29,2,8, SRA_R(C)               ) \
y(0x6, 0x0, 0x2A,2,8, SRA_R(D)               ) \
y(0x6, 0x0, 0x2B,2,8, SRA_R(E)               ) \
y(0x6, 0x0, 0x2C,2,8, SRA_R(H)               ) \
y(0x6, 0x0, 0x2D,2,8, SRA_R(L)               ) \
y(0x6, 0x0, 0x2E,2,15,SRA_D(HL_p)                ) \
y(0x6, 0x0, 0x2F,2,8, SRA_R(A)               ) \
y(0x6, 0x0, 0x30,2,8, SWAP_R(B)               ) \
y(0x6, 0x0, 0x31,2,8, SWAP_R(C)               ) \
y(0x6, 0x0, 0x32,2,8, SWAP_R(D)               ) \
y(0x6, 0x0, 0x33,2,8, SWAP_R(E)               ) \
y(0x6, 0x0, 0x34,2,8, SWAP_R(H)               ) \
y(0x6, 0x0, 0x35,2,8, SWAP_R(L)               ) \
y(0x6, 0x0, 0x36,2,15,SWAP_D(HL_p)                ) \
y(0x6, 0x0, 0x37,2,8, SWAP_R(A)               ) \
y(0x6, 0x0, 0x38,2,8, SWAP_R(B)               ) \
y(0x6, 0x0, 0x39,2,8, SWAP_R(C)               ) \
y(0x6, 0x0, 0x3A,2,8, SWAP_R(D)               ) \
y(0x6, 0x0, 0x3B,2,8, SWAP_R(E)               ) \
y(0x6, 0x0, 0x3C,2,8, SWAP_R(H)               ) \
y(0x6, 0x0, 0x3D,2,8, SWAP_R(L)               ) \
y(0x6, 0x0, 0x3E,2,15,SWAP_D(HL_p)                ) \
y(0x6, 0x0, 0x3F,2,8, SWAP_R(A)               ) \
y(0x4, 0x2, 0x40,2,8, BITN_R(0, B)               ) \
y(0x4, 0x2, 0x41,2,8, BITN_R(0, C)               ) \
y(0x4, 0x2, 0x42,2,8, BITN_R(0, D)               ) \
y(0x4, 0x2, 0x43,2,8, BITN_R(0, E)               ) \
y(0x4, 0x2, 0x44,2,8, BITN_R(0, H)               ) \
y(0x4, 0x2, 0x45,2,8, BITN_R(0, L)               ) \
y(0x4, 0x2, 0x46,2,12,BITN_D(0, HL_p)                ) \
y(0x4, 0x2, 0x47,2,8, BITN_R(0, A)               ) \
y(0x4, 0x2, 0x48,2,8, BITN_R(1, B)               ) \
y(0x4, 0x2, 0x49,2,8, BITN_R(1, C)               ) \
y(0x4, 0x2, 0x4A,2,8, BITN_R(1, D)               ) \
y(0x4, 0x2, 0x4B,2,8, BITN_R(1, E)               ) \
y(0x4, 0x2, 0x4C,2,8, BITN_R(1, H)               ) \
y(0x4, 0x2, 0x4D,2,8, BITN_R(1, L)               ) \
y(0x4, 0x2, 0x4E,2,12,BITN_D(1, HL_p)                ) \
y(0x4, 0x2, 0x4F,2,8, BITN_R(1, A)               ) \
y(0x4, 0x2, 0x50,2,8, BITN_R(2, B)               ) \
y(0x4, 0x2, 0x51,2,8, BITN_R(2, C)               ) \
y(0x4, 0x2, 0x52,2,8, BITN_R(2, D)               ) \
y(0x4, 0x2, 0x53,2,8, BITN_R(2, E)               ) \
y(0x4, 0x2, 0x54,2,8, BITN_R(2, H)               ) \
y(0x4, 0x2, 0x55,2,8, BITN_R(2, L)               ) \
y(0x4, 0x2, 0x56,2,12,BITN_D(2, HL_p)                ) \
y(0x4, 0x2, 0x57,2,8, BITN_R(2, A)               ) \
y(0x4, 0x2, 0x58,2,8, BITN_R(3, B)               ) \
y(0x4, 0x2, 0x59,2,8, BITN_R(3, C)               ) \
y(0x4, 0x2, 0x5A,2,8, BITN_R(3, D)               ) \
y(0x4, 0x2, 0x5B,2,8, BITN_R(3, E)               ) \
y(0x4, 0x2, 0x5C,2,8, BITN_R(3, H)               ) \
y(0x4, 0x2, 0x5D,2,8, BITN_R(3, L)               ) \
y(0x4, 0x2, 0x5E,2,12,BITN_D(3, HL_p)                ) \
y(0x4, 0x2, 0x5F,2,8, BITN_R(3, A)               ) \
y(0x4, 0x2, 0x60,2,8, BITN_R(4, B)               ) \
y(0x4, 0x2, 0x61,2,8, BITN_R(4, C)               ) \
y(0x4, 0x2, 0x62,2,8, BITN_R(4, D)               ) \
y(0x4, 0x2, 0x63,2,8, BITN_R(4, E)               ) \
y(0x4, 0x2, 0x64,2,8, BITN_R(4, H)               ) \
y(0x4, 0x2, 0x65,2,8, BITN_R(4, L)               ) \
y(0x4, 0x2, 0x66,2,12,BITN_D(4, HL_p)                ) \
y(0x4, 0x2, 0x67,2,8, BITN_R(4, A)               ) \
y(0x4, 0x2, 0x68,2,8, BITN_R(5, B)               ) \
y(0x4, 0x2, 0x69,2,8, BITN_R(5, C)               ) \
y(0x4, 0x2, 0x6A,2,8, BITN_R(5, D)               ) \
y(0x4, 0x2, 0x6B,2,8, BITN_R(5, E)               ) \
y(0x4, 0x2, 0x6C,2,8, BITN_R(5, H)               ) \
y(0x4, 0x2, 0x6D,2,8, BITN_R(5, L)               ) \
y(0x4, 0x2, 0x6E,2,12,BITN_D(5, HL_p)                ) \
y(0x4, 0x2, 0x6F,2,8, BITN_R(5, A)               ) \
y(0x4, 0x2, 0x70,2,8, BITN_R(6, B)               ) \
y(0x4, 0x2, 0x71,2,8, BITN_R(6, C)               ) \
y(0x4, 0x2, 0x72,2,8, BITN_R(6, D)               ) \
y(0x4, 0x2, 0x73,2,8, BITN_R(6, E)               ) \
y(0x4, 0x2, 0x74,2,8, BITN_R(6, H)               ) \
y(0x4, 0x2, 0x75,2,8, BITN_R(6, L)               ) \
y(0x4, 0x2, 0x76,2,12,BITN_D(6, HL_p)                ) \
y(0x4, 0x2, 0x77,2,8, BITN_R(6, A)               ) \
y(0x4, 0x2, 0x78,2,8, BITN_R(7, B)               ) \
y(0x4, 0x2, 0x79,2,8, BITN_R(7, C)               ) \
y(0x4, 0x2, 0x7A,2,8, BITN_R(7, D)               ) \
y(0x4, 0x2, 0x7B,2,8, BITN_R(7, E)               ) \
y(0x4, 0x2, 0x7C,2,8, BITN_R(7, H)               ) \
y(0x4, 0x2, 0x7D,2,8, BITN_R(7, L)               ) \
y(0x4, 0x2, 0x7E,2,12,BITN_D(7, HL_p)                ) \
y(0x4, 0x2, 0x7F,2,8, BITN_R(7, A)               ) \
y(0x0, 0x0, 0x80,2,8, resetBitN_R(0, B)               ) \
y(0x0, 0x0, 0x81,2,8, resetBitN_R(0, C)               ) \
y(0x0, 0x0, 0x82,2,8, resetBitN_R(0, D)               ) \
y(0x0, 0x0, 0x83,2,8, resetBitN_R(0, E)               ) \
y(0x0, 0x0, 0x84,2,8, resetBitN_R(0, H)               ) \
y(0x0, 0x0, 0x85,2,8, resetBitN_R(0, L)               ) \
y(0x0, 0x0, 0x86,2,15,resetBitN_D(0, HL_p)                ) \
y(0x0, 0x0, 0x87,2,8, resetBitN_R(0, A)               ) \
y(0x0, 0x0, 0x88,2,8, resetBitN_R(1, B)               ) \
y(0x0, 0x0, 0x89,2,8, resetBitN_R(1, C)               ) \
y(0x0, 0x0, 0x8A,2,8, resetBitN_R(1, D)               ) \
y(0x0, 0x0, 0x8B,2,8, resetBitN_R(1, E)               ) \
y(0x0, 0x0, 0x8C,2,8, resetBitN_R(1, H)               ) \
y(0x0, 0x0, 0x8D,2,8, resetBitN_R(1, L)               ) \
y(0x0, 0x0, 0x8E,2,15,resetBitN_D(1, HL_p)                ) \
y(0x0, 0x0, 0x8F,2,8, resetBitN_R(1, A)               ) \
y(0x0, 0x0, 0x90,2,8, resetBitN_R(2, B)               ) \
y(0x0, 0x0, 0x91,2,8, resetBitN_R(2, C)               ) \
y(0x0, 0x0, 0x92,2,8, resetBitN_R(2, D)               ) \
y(0x0, 0x0, 0x93,2,8, resetBitN_R(2, E)               ) \
y(0x0, 0x0, 0x94,2,8, resetBitN_R(2, H)               ) \
y(0x0, 0x0, 0x95,2,8, resetBitN_R(2, L)               ) \
y(0x0, 0x0, 0x96,2,15,resetBitN_D(2, HL_p)                ) \
y(0x0, 0x0, 0x97,2,8, resetBitN_R(2, A)               ) \
y(0x0, 0x0, 0x98,2,8, resetBitN_R(3, B)               ) \
y(0x0, 0x0, 0x99,2,8, resetBitN_R(3, C)               ) \
y(0x0, 0x0, 0x9A,2,8, resetBitN_R(3, D)               ) \
y(0x0, 0x0, 0x9B,2,8, resetBitN_R(3, E)               ) \
y(0x0, 0x0, 0x9C,2,8, resetBitN_R(3, H)               ) \
y(0x0, 0x0, 0x9D,2,8, resetBitN_R(3, L)               ) \
y(0x0, 0x0, 0x9E,2,15,resetBitN_D(3, HL_p)                ) \
y(0x0, 0x0, 0x9F,2,8, resetBitN_R(3, A)               ) \
y(0x0, 0x0, 0xA0,2,8, resetBitN_R(4, B)               ) \
y(0x0, 0x0, 0xA1,2,8, resetBitN_R(4, C)               ) \
y(0x0, 0x0, 0xA2,2,8, resetBitN_R(4, D)               ) \
y(0x0, 0x0, 0xA3,2,8, resetBitN_R(4, E)               ) \
y(0x0, 0x0, 0xA4,2,8, resetBitN_R(4, H)               ) \
y(0x0, 0x0, 0xA5,2,8, resetBitN_R(4, L)               ) \
y(0x0, 0x0, 0xA6,2,15,resetBitN_D(4, HL_p)                ) \
y(0x0, 0x0, 0xA7,2,8, resetBitN_R(4, A)               ) \
y(0x0, 0x0, 0xA8,2,8, resetBitN_R(5, B)               ) \
y(0x0, 0x0, 0xA9,2,8, resetBitN_R(5, C)               ) \
y(0x0, 0x0, 0xAA,2,8, resetBitN_R(5, D)               ) \
y(0x0, 0x0, 0xAB,2,8, resetBitN_R(5, E)               ) \
y(0x0, 0x0, 0xAC,2,8, resetBitN_R(5, H)               ) \
y(0x0, 0x0, 0xAD,2,8, resetBitN_R(5, L)               ) \
y(0x0, 0x0, 0xAE,2,15,resetBitN_D(5, HL_p)                ) \
y(0x0, 0x0, 0xAF,2,8, resetBitN_R(5, A)               ) \
y(0x0, 0x0, 0xB0,2,8, resetBitN_R(6, B)               ) \
y(0x0, 0x0, 0xB1,2,8, resetBitN_R(6, C)               ) \
y(0x0, 0x0, 0xB2,2,8, resetBitN_R(6, D)               ) \
y(0x0, 0x0, 0xB3,2,8, resetBitN_R(6, E)               ) \
y(0x0, 0x0, 0xB4,2,8, resetBitN_R(6, H)               ) \
y(0x0, 0x0, 0xB5,2,8, resetBitN_R(6, L)               ) \
y(0x0, 0x0, 0xB6,2,15,resetBitN_D(6, HL_p)                ) \
y(0x0, 0x0, 0xB7,2,8, resetBitN_R(6, A)               ) \
y(0x0, 0x0, 0xB8,2,8, resetBitN_R(7, B)               ) \
y(0x0, 0x0, 0xB9,2,8, resetBitN_R(7, C)               ) \
y(0x0, 0x0, 0xBA,2,8, resetBitN_R(7, D)               ) \
y(0x0, 0x0, 0xBB,2,8, resetBitN_R(7, E)               ) \
y(0x0, 0x0, 0xBC,2,8, resetBitN_R(7, H)               ) \
y(0x0, 0x0, 0xBD,2,8, resetBitN_R(7, L)               ) \
y(0x0, 0x0, 0xBE,2,15,resetBitN_D(7, HL_p)                ) \
y(0x0, 0x0, 0xBF,2,8, resetBitN_R(7, A)               ) \
y(0x0, 0x0, 0xC0,2,8, setBitN_R(0,B)  ) \
y(0x0, 0x0, 0xC1,2,8, setBitN_R(0,C)         ) \
y(0x0, 0x0, 0xC2,2,8, setBitN_R(0,D)         ) \
y(0x0, 0x0, 0xC3,2,8, setBitN_R(0,E)         ) \
y(0x0, 0x0, 0xC4,2,8, setBitN_R(0,H)         ) \
y(0x0, 0x0, 0xC5,2,8, setBitN_R(0,L)         ) \
y(0x0, 0x0, 0xC6,2,15,setBitN_D(0,HL_p)        ) \
y(0x0, 0x0, 0xC7,2,8, setBitN_R(0,A)		) \
y(0x0, 0x0, 0xC8,2,8, setBitN_R(1,B)		) \
y(0x0, 0x0, 0xC9,2,8, setBitN_R(1,C)		) \
y(0x0, 0x0, 0xCA,2,8, setBitN_R(1,D)		) \
y(0x0, 0x0, 0xCB,2,8, setBitN_R(1,E)		) \
y(0x0, 0x0, 0xCC,2,8, setBitN_R(1,H)		) \
y(0x0, 0x0, 0xCD,2,8, setBitN_R(1,L)		) \
y(0x0, 0x0, 0xCE,2,15,setBitN_D(1,HL_p)		) \
y(0x0, 0x0, 0xCF,2,8, setBitN_R(1,A)		) \
y(0x0, 0x0, 0xD0,2,8, setBitN_R(2,B)		) \
y(0x0, 0x0, 0xD1,2,8, setBitN_R(2,C)		) \
y(0x0, 0x0, 0xD2,2,8, setBitN_R(2,D)		) \
y(0x0, 0x0, 0xD3,2,8, setBitN_R(2,E)		) \
y(0x0, 0x0, 0xD4,2,8, setBitN_R(2,H)		) \
y(0x0, 0x0, 0xD5,2,8, setBitN_R(2,L)		) \
y(0x0, 0x0, 0xD6,2,15,setBitN_D(2,HL_p)		) \
y(0x0, 0x0, 0xD7,2,8, setBitN_R(2,A)		) \
y(0x0, 0x0, 0xD8,2,8, setBitN_R(3,B)		) \
y(0x0, 0x0, 0xD9,2,8, setBitN_R(3,C)		) \
y(0x0, 0x0, 0xDA,2,8, setBitN_R(3,D)		) \
y(0x0, 0x0, 0xDB,2,8, setBitN_R(3,E)		) \
y(0x0, 0x0, 0xDC,2,8, setBitN_R(3,H)		) \
y(0x0, 0x0, 0xDD,2,8, setBitN_R(3,L)		) \
y(0x0, 0x0, 0xDE,2,15,setBitN_D(3,HL_p)		) \
y(0x0, 0x0, 0xDF,2,8, setBitN_R(3,A)		) \
y(0x0, 0x0, 0xE0,2,8, setBitN_R(4,B)		) \
y(0x0, 0x0, 0xE1,2,8, setBitN_R(4,C)		) \
y(0x0, 0x0, 0xE2,2,8, setBitN_R(4,D)		) \
y(0x0, 0x0, 0xE3,2,8, setBitN_R(4,E)		) \
y(0x0, 0x0, 0xE4,2,8, setBitN_R(4,H)		) \
y(0x0, 0x0, 0xE5,2,8, setBitN_R(4,L)		) \
y(0x0, 0x0, 0xE6,2,15,setBitN_D(4,HL_p)		) \
y(0x0, 0x0, 0xE7,2,8, setBitN_R(4,A)		) \
y(0x0, 0x0, 0xE8,2,8, setBitN_R(5,B)		) \
y(0x0, 0x0, 0xE9,2,8, setBitN_R(5,C)		) \
y(0x0, 0x0, 0xEA,2,8, setBitN_R(5,D)		) \
y(0x0, 0x0, 0xEB,2,8, setBitN_R(5,E)		) \
y(0x0, 0x0, 0xEC,2,8, setBitN_R(5,H)		) \
y(0x0, 0x0, 0xED,2,8, setBitN_R(5,L)		) \
y(0x0, 0x0, 0xEE,2,15,setBitN_D(5,HL_p)		) \
y(0x0, 0x0, 0xEF,2,8, setBitN_R(5,A)		) \
y(0x0, 0x0, 0xF0,2,8, setBitN_R(6,B)		) \
y(0x0, 0x0, 0xF1,2,8, setBitN_R(6,C)		) \
y(0x0, 0x0, 0xF2,2,8, setBitN_R(6,D)		) \
y(0x0, 0x0, 0xF3,2,8, setBitN_R(6,E)		) \
y(0x0, 0x0, 0xF4,2,8, setBitN_R(6,H)		) \
y(0x0, 0x0, 0xF5,2,8, setBitN_R(6,L)		) \
y(0x0, 0x0, 0xF6,2,15,setBitN_D(6,HL_p)		) \
y(0x0, 0x0, 0xF7,2,8, setBitN_R(6,A)		) \
y(0x0, 0x0, 0xF8,2,8, setBitN_R(7,B)		) \
y(0x0, 0x0, 0xF9,2,8, setBitN_R(7,C)		) \
y(0x0, 0x0, 0xFA,2,8, setBitN_R(7,D)		) \
y(0x0, 0x0, 0xFB,2,8, setBitN_R(7,E)		) \
y(0x0, 0x0, 0xFC,2,8, setBitN_R(7,H)		) \
y(0x0, 0x0, 0xFD,2,8, setBitN_R(7,L)		) \
y(0x0, 0x0, 0xFE,2,15,setBitN_D(7,HL_p)		) \
y(0x0, 0x0, 0xFF,2,8, setBitN_R(7,A)		)

//  End of INSTRUCTIONS_CB(y)

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
			if(RAM::read(0xFF01) != 0x0){
				std::cout << "Serial data: " << Math::decHex(RAM::read(0xFF01)) << "\n";
			}
			
			cycle();
			//std::cout << cycles << "\n";
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
				#define x(unsetMask,setMask,op,byte,cycle,ins) case op:{ ins; Registers.F |= setMask; cycles += cycle; Registers.PC += byte; break;}
				INSTRUCTIONS(x)
				#undef x

				default: std::cout << "UNKNOWN OPCODE: " << Math::decHex(prefix) << "\n";break;
			}
		} else {
			switch(opcode){
				#define y(unsetMask,setMask,op,byte,cycle,ins) case op:{ ins; Registers.F |= setMask; cycles += cycle; Registers.PC += byte; break; }
				INSTRUCTIONS_CB(y)
				#undef y

				default: std::cout << "UNKNOWN CB OPCODE: " << Math::decHex(opcode) << "\n";
			}
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

	inline void copyToC(int mask){
		if(readC != mask){
			Registers.F ^= 0x10;
		}
	}
}  // namespace CPU

