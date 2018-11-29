#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include "CPU.hpp"
#include "../Math/Math.hpp"
#include "../Memory/RAM.hpp"
#include "../Debug/Debug.hpp"
#include "../Config/Config.hpp"
#include "PPU.hpp"
/* 
	Flag reading
*/
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

/*  
	Modifying bits
*/
#define setBitN_R(a, b)  	(Registers.b |= (0x01 << a))
#define setBitN_D(a, b)  	(RAM::write(b, RAM::read(b) | (0x01 << a)))
#define resetBitN_R(a, b)   (Registers.b &= ~(1 << a))
#define resetBitN_D(a, b)   (RAM::write(b, RAM::read(b) & ~(1 << a)))
#define SWAP_R(a) 			(Registers.a = (Registers.a >> 4) | (Registers.a << 4)); if(Registers.a == 0){ setZ; } else { unsetZ; }
#define SWAP_D(a)			RAM::write(a, (RAM::read(a) >> 4) | (RAM::read(a) << 4)); if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define SLA_R(a)	copyToC(Registers.a >> 7); \
					Registers.a = Registers.a << 1; \
					Registers.a &= ~(1); \
					if(Registers.a == 0){ setZ; } else { unsetZ; }
					
#define SLA_D(a)	copyToC(RAM::read(a) >> 7); \
					RAM::write(a, RAM::read(a) << 1); \
					RAM::write(a, RAM::read(a) & ~(1)); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define SRA_R(a)	copyToC(Registers.a & 1); \
					Registers.a = (Registers.a >> 1) | (Registers.a & 0x80); \
					if(Registers.a == 0){ setZ; } else { unsetZ; }

#define SRA_D(a)	copyToC(RAM::read(a) & 1); \
					RAM::write(a, (RAM::read(a) >> 1) | (RAM::read(a) & 0x80)); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define SRL_R(a)	copyToC(Registers.a & 1); \
					Registers.a = (Registers.a >> 1); \
					if (Registers.a == 0) { setZ; } else { unsetZ; }

#define SRL_D(a)	copyToC(RAM::read(a) & 1); \
					RAM::write(a, (RAM::read(a) >> 1)); \
					if (RAM::read(a) == 0) { setZ; } else { unsetZ; } 

#define RL_R(a)		int temp = readC, bit7 = Registers.a >> 7; \
					copyToC(bit7); \
					Registers.a = Registers.a << 1; \
					Registers.a = ((Registers.a & (~1) ) | temp); \
					if(Registers.a == 0){ setZ; } else { unsetZ; }

#define RR_R(a)		int temp = readC, bit0 = Registers.a & 1; \
					copyToC(bit0); \
					Registers.a = Registers.a >> 1; \
					Registers.a = (Registers.a & (~(1 << 7))) | (temp << 7); \
					if(Registers.a == 0){ setZ; } else { unsetZ; }

#define RL_D(a)		int temp = readC, bit7 = RAM::read(a) >> 7; \
					copyToC(bit7); \
					RAM::write(a, RAM::read(a) << 1 ); \
					RAM::write(a, ( RAM::read(a) & (~1) )| temp); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define RR_D(a)		int temp = readC, bit0 = RAM::read(a) & 1; \
					copyToC(bit0); \
					RAM::write(a, RAM::read(a) >> 1); \
					RAM::write(a, ( RAM::read(a) & (~(1 << 7)) ) | (temp << 7)); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define RLC_R(a)	int bit7 = Registers.a >> 7; \
					copyToC(bit7); \
					Registers.a = Registers.a << 1; \
					Registers.a = (Registers.a & (~1)) | bit7; \
					if(Registers.a == 0){ setZ; } else { unsetZ; }

#define RRC_R(a)	int bit0 = Registers.a & 1; \
					copyToC(bit0); \
					Registers.a = Registers.a >> 1; \
					Registers.a = (Registers.a & ~(1 << 7)) | (bit0 << 7); \
					if(Registers.a == 0){ setZ; } else { unsetZ; }

#define RLC_D(a)	int bit7 = RAM::read(a) >> 7; \
					copyToC(bit7); \
					RAM::write(a, RAM::read(a) << 1); \
					RAM::write(a, (RAM::read(a) & (~1)) | bit7); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

#define RRC_D(a)	int bit0 = RAM::read(a) & 1; \
					copyToC(bit0); \
					RAM::write(a, RAM::read(a) >> 1); \
					RAM::write(a, (RAM::read(a) & ~(1 << 7)) | (bit0 << 7)); \
					if(RAM::read(a) == 0){ setZ; } else { unsetZ; }

/*  
	Bit testing
*/
#define BIT_X_R(a,b) (Registers.b & (1 << a))
#define BIT_X_D(a,b) (b & (1 << a))

#define BITN_R(a,b)  if(!BIT_X_R(a,b)){ setZ; } else { unsetZ; }
#define BITN_D(a,b)  if(!BIT_X_D(a,b)){ setZ; } else { unsetZ; }

/*
  	Load / store instructions and flow control
*/
#define LD_R_R(a, b) CPU::Registers.a = CPU::Registers.b
#define LD_R_D(a, b) CPU::Registers.a = b
#define LD_D_R(a, b) RAM::write(a, CPU::Registers.b)
//#define LD_D16_R(a, b) RAM::write_16(a, CPU::Registers.b)
#define LDI_D_R(a, b) RAM::write(a, CPU::Registers.b); writeHL(readHL() + 1)
#define LDI_R_D(a, b) CPU::Registers.a = b; writeHL(readHL() + 1)
#define LDD_D_R(a, b) RAM::write(a, CPU::Registers.b); writeHL(readHL() - 1)
#define LDD_R_D(a, b) CPU::Registers.a = b; writeHL(readHL() - 1)
#define JR_CC_N(a, b) if(a){ \
						CPU::Registers.PC += (int8_t)b; \
					  }	

#define JR_N(a)	CPU::Registers.PC += a; 

#define JP_N(a) CPU::Registers.PC = a; preservePC = true;

#define JP_CC_N(a, b) if(a){ CPU::Registers.PC = b; preservePC = true;}

#define CALL_NN(a)  RAM::write_16(CPU::Registers.SP - 0x02, ((CPU::Registers.PC + 0x03) >> 8) | ((CPU::Registers.PC + 0x03) << 8) ); \
					CPU::Registers.PC = a;	\
					CPU::Registers.SP -= 0x02; \
					preservePC = true;

#define CALL_XX_NN(a, b) if(a){ CALL_NN(b); }


#define RET() 		CPU::Registers.PC = RAM::read(CPU::Registers.SP) | RAM::read(CPU::Registers.SP + 0x1)*0x100;	\
					CPU::Registers.SP += 0x02; \
					preservePC = true;


#define RET_XX(a)		if(a){ RET(); }

#define RETI() 		CPU::Registers.IME = true; \
					RET();

#define PUSH_NN(a)	CPU::Registers.SP -= 0x02; \
					RAM::write_16(CPU::Registers.SP, (a << 8) | (a >> 8));

#define POP_NN(a)	write##a(RAM::read(CPU::Registers.SP) | RAM::read(CPU::Registers.SP + 0x1)*0x100); \
					CPU::Registers.SP += 0x02;

#define RST_N(a)	CPU::Registers.SP -= 0x02; \
					RAM::write_16(CPU::Registers.SP, ((CPU::Registers.PC + 0x1) << 8) | ((CPU::Registers.PC + 0x1) >> 8)); \
					CPU::Registers.PC = 0x0 + a; \
					preservePC = true;

/*
	Arithmetic
*/
#define ADD_R_R(a, b) if(CPU::Registers.a + CPU::Registers.b > 0xFF){ setC; } else { unsetC; }; \
					  if( (CPU::Registers.a & 0x0F) + (CPU::Registers.b & 0x0F) > 0x0F){ setH; } else { unsetH; } \
					  CPU::Registers.a += CPU::Registers.b; \
					  if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }

#define ADD_R_D(a, b) if(CPU::Registers.a + b > 0xFF){ setC; } else { unsetC; }; \
					  if( (CPU::Registers.a & 0x0F) + (b & 0x0F) > 0x0F){ setH; } else { unsetH; } \
					  CPU::Registers.a += b; \
					  if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }

#define ADC_R(a) char16_t prev = CPU::Registers.A; \
				 char16_t data = CPU::Registers.a; \
				 CPU::Registers.A = CPU::Registers.A + data + readC; \
				 if( (prev & 0xF) + (data & 0xF) + readC > 0xF){ setH; } else { unsetH; } \
				 if(CPU::Registers.A == 0){ setZ; } else { unsetZ; } \
				 if(prev + data + readC > 0xFF){ setC; } else { unsetC; }

#define ADC_D(a) char16_t prev = CPU::Registers.A; \
				 CPU::Registers.A = CPU::Registers.A + a + readC; \
				 if( (prev & 0xF) + (a & 0xF) + readC > 0xF){ setH; } else { unsetH; } \
				 if(CPU::Registers.A == 0){ setZ; } else { unsetZ; } \
				 if(prev + a + readC > 0xFF){ setC; } else { unsetC; };

#define INC_R(a) if( (CPU::Registers.a & 0x0F) + (1 & 0x0F) > 0x0F){ setH; } else { unsetH; } \
				 CPU::Registers.a += 1; \
				 if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }

#define DEC_R(a) if( (CPU::Registers.a & 0x0F) - (1 & 0x0F) < 0){ setH; } else { unsetH; } \
				 CPU::Registers.a -= 1; \
				 if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }

 
#define SUB_R_R(a,b) if(CPU::Registers.a < CPU::Registers.b){ setC; } else { unsetC; }; \
				 	 if( (CPU::Registers.a & 0x0F) - (CPU::Registers.b & 0x0F) < 0){ setH; } else { unsetH; } \
				     CPU::Registers.a -= CPU::Registers.b; \
				 	 if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }

#define SUB_R_D(a,b) if(CPU::Registers.a < b){ setC; } else { unsetC; }; \
				 	 if( (CPU::Registers.a & 0x0F) - (b & 0x0F) < 0){ setH; } else { unsetH; } \
				 	 CPU::Registers.a -= b; \
				 	 if(CPU::Registers.a == 0){ setZ; } else { unsetZ; }



#define SBC_R(a) char16_t prev = CPU::Registers.A; \
				 CPU::Registers.A = CPU::Registers.A - CPU::Registers.a - readC; \
				 if(CPU::Registers.A == 0){ setZ; } else { unsetZ; } \
				 if( (prev & 0x0F) - (CPU::Registers.a & 0x0F) - readC < 0){ setH; } else { unsetH; } \
				 if(prev < (CPU::Registers.a + readC)){ setC; } else { unsetC; }

#define SBC_D(a) char16_t prev = CPU::Registers.A; \
				 CPU::Registers.A = CPU::Registers.A - a - readC; \
				 if(CPU::Registers.A == 0){ setZ; } else { unsetZ; } \
				 if( (prev & 0x0F) - (a & 0x0F) - readC < 0){ setH; } else { unsetH; } \
				 if(prev < (a + readC)){ setC; } else { unsetC; } \

#define CP_R(a)	if(CPU::Registers.A - CPU::Registers.a == 0){ setZ; } else { unsetZ; } \
				if(((CPU::Registers.A & 0x0F) - (CPU::Registers.a & 0x0F) < 0)){ setH; } else { unsetH; } \
				if(CPU::Registers.A < CPU::Registers.a){ setC; } else { unsetC; }

#define CP_D(a) if(CPU::Registers.A - a == 0){ setZ; } else { unsetZ; } \
				if(((CPU::Registers.A & 0x0F) - (a & 0x0F) < 0)){ setH; } else { unsetH; } \
				if(CPU::Registers.A < a){ setC; } else { unsetC; }
						
#define ADD_HL_N(a)	if(readHL() + a > 0xFFFF){ setC; } else { unsetC; } \
					if( (readHL() & 0xFFF) + (a & 0xFFF) > 0xFFF){ setH; } else { unsetH; } \
				    writeHL(readHL() + a);

#define INC_N(a) RAM::write(a, RAM::read(a) + 1); \
				 if(RAM::read(a) == 0){ setZ; } else { unsetZ; } \
				 if(((RAM::read(a)) & 0xF) == 0){ setH; } else { unsetH; } \
				 

#define DEC_N(a) if(RAM::read(a) - 1 == 0){ setZ; } else { unsetZ; } \
				 if((RAM::read(a) & 0x0F) - (1 & 0x0F) < 0){ setH; } else { unsetH; } \
				 RAM::write(a, RAM::read(a) - 1);

/*
  Bitwise operations
*/
#define AND_R(a) CPU::Registers.A &= CPU::Registers.a;  if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }
#define AND_D(a) CPU::Registers.A &= a; 				if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }
#define OR_R(a) CPU::Registers.A |= CPU::Registers.a;   if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }
#define OR_D(a) CPU::Registers.A |= a; 					if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }
#define XOR_R(a) CPU::Registers.A ^= CPU::Registers.a;  if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }
#define XOR_D(a) CPU::Registers.A ^= a; 				if(CPU::Registers.A == 0){ setZ; } else { unsetZ; }

/* 
	Values in memory pointed to by xx
*/
#define BC_mv RAM::read(CPU::readBC())
#define DE_mv RAM::read(CPU::readDE())
#define HL_mv RAM::read(CPU::readHL())

/* 
    Values of xx
*/
#define BC_v  CPU::readBC()
#define DE_v  CPU::readDE()
#define HL_v  CPU::readHL()

//  Values of C/H are computed on the lower nibble only
#define LD_HL_SPR8()	if( ((int8_t)(char)(CPU::Registers.SP) & 0xFF)        +  ((int8_t)(char)r8 & 0xFF) > 0xFF){ setC; } else { unsetC; } \
						if( ((int8_t)(char)(CPU::Registers.SP) & 0xFF & 0xF) + ((int8_t)(char)r8 & 0xFF & 0xF) > 0xF){ setH; } else { unsetH; } \
						writeHL(CPU::Registers.SP + (int8_t)(char)r8);

//  Values of C/H are computed on the lower nibble only
#define ADD_SP_R8()     if( (CPU::Registers.SP & 0xFF) + ((int8_t)(char)r8 & 0xFF) > 0xFF){ setC; } else { unsetC; } \
						if( (CPU::Registers.SP & 0xFF & 0xF) + ((int8_t)(char)r8 & 0xFF & 0xF) > 0xF){ setH; } else { unsetH; } \
						CPU::Registers.SP += (int8_t)(char)r8; \

/* 
	!!!!! CAUTION !!!!!
	MACRO FUCKERY AHEAD

	x( F bits disabled mask, F bits enabled mask, OPcode, Byte count, Cycle count, Expression/Action)
*/
#define INSTRUCTIONS(x) \
x(0x0, 0x0, 0x00,1,4, /* filler */ 										) \
x(0x0, 0x0, 0x01,3,12,writeBC(d16)   									) \
x(0x0, 0x0, 0x02,1,8,LD_D_R(BC_v, A) 									) \
x(0x0, 0x0, 0x03,1,8,writeBC(readBC() + 0x01)							) \
x(0x4, 0x0, 0x04,1,4,INC_R(B)		   									) \
x(0x0, 0x4, 0x05,1,4,DEC_R(B)        									) \
x(0x0, 0x0, 0x06,2,8,LD_R_D(B, d8)   									) \
x(0xE, 0x0, 0x07,1,4,RLC_R(A) 											) \
x(0x0, 0x0, 0x08,3,20,RAM::write_16(a16, (CPU::Registers.SP << 8) | (CPU::Registers.SP >> 8)) ) \
x(0x4, 0x0, 0x09,1,8,ADD_HL_N(readBC())									) \
x(0x0, 0x0, 0x0A,1,8,LD_R_D(A, BC_mv) 									) \
x(0x0, 0x0, 0x0B,1,8,writeBC(readBC() - 0x01)							) \
x(0x4, 0x0, 0x0C,1,4,INC_R(C)        									) \
x(0x0, 0x4, 0x0D,1,4,DEC_R(C)		   									) \
x(0x0, 0x0, 0x0E,2,8,LD_R_D(C, d8)   									) \
x(0xE, 0x0, 0x0F,1,4,RRC_R(A)			   								) \
x(0x0, 0x0, 0x10,2,4,		/*  stop  */	   						) \
x(0x0, 0x0, 0x11,3,12,writeDE(d16)   									) \
x(0x0, 0x0, 0x12,1,8,LD_D_R(DE_v, A) 									) \
x(0x0, 0x0, 0x13,1,8,writeDE(readDE() + 0x01)                			) \
x(0x4, 0x0, 0x14,1,4,INC_R(D)        									) \
x(0x0, 0x4, 0x15,1,4,DEC_R(D)        									) \
x(0x0, 0x0, 0x16,2,8,LD_R_D(D, d8)   									) \
x(0xE, 0x0, 0x17,1,4,RL_R(A) 			   								) \
x(0x0, 0x0, 0x18,2,12,JR_N(r8) 											) \
x(0x4, 0x0, 0x19,1,8,ADD_HL_N(readDE()) 								) \
x(0x0, 0x0, 0x1A,1,8,LD_R_D(A, DE_mv) 									) \
x(0x0, 0x0, 0x1B,1,8,writeDE(readDE() - 0x01)							) \
x(0x4, 0x0, 0x1C,1,4,INC_R(E)        									) \
x(0x0, 0x4, 0x1D,1,4,DEC_R(E)        									) \
x(0x0, 0x0, 0x1E,2,8,LD_R_D(E, d8)   									) \
x(0xE, 0x0, 0x1F,1,4,RR_R(A)                							) \
x(0x0, 0x0, 0x20,2,8,JR_CC_N(!readZ, (char)r8) 								) \
x(0x0, 0x0, 0x21,3,12,writeHL(d16)   									) \
x(0x0, 0x0, 0x22,1,8,LDI_D_R(HL_v, A) 									) \
x(0x0, 0x0, 0x23,1,8,writeHL(readHL() + 1)                				) \
x(0x4, 0x0, 0x24,1,4,INC_R(H)        									) \
x(0x0, 0x4, 0x25,1,4,DEC_R(H)        									) \
x(0x0, 0x0, 0x26,2,8,LD_R_D(H, d8)   									) \
x(0x2, 0x0, 0x27,1,4,DAA()		             							) \
x(0x0, 0x0, 0x28,2,8,JR_CC_N(readZ, (char)r8)               					) \
x(0x4, 0x0, 0x29,1,8,ADD_HL_N(readHL())                					) \
x(0x0, 0x0, 0x2A,1,8,LDI_R_D(A, HL_mv)            						) \
x(0x0, 0x0, 0x2B,1,8,writeHL(readHL() - 0x1 )                			) \
x(0x4, 0x0, 0x2C,1,4,INC_R(L)		   									) \
x(0x0, 0x4, 0x2D,1,4,DEC_R(L)        									) \
x(0x0, 0x0, 0x2E,2,8,LD_R_D(L, d8)   									) \
x(0x0, 0x6, 0x2F,1,4, (CPU::Registers.A = ~CPU::Registers.A)    		) \
x(0x0, 0x0, 0x30,2,8,JR_CC_N(!readC, (char)r8) 			   					) \
x(0x0, 0x0, 0x31,3,12,LD_R_D(SP, d16)									) \
x(0x0, 0x0, 0x32,1,8,LDD_D_R(HL_v, A)           						) \
x(0x0, 0x0, 0x33,1,8,CPU::Registers.SP += 0x1							) \
x(0x4, 0x0, 0x34,1,12,INC_N(HL_v)			 							) \
x(0x0, 0x4, 0x35,1,12,DEC_N(HL_v) 										) \
x(0x0, 0x0, 0x36,2,12,RAM::write(HL_v, d8)								) \
x(0x6, 0x1, 0x37,1,4,setC           									) \
x(0x0, 0x0, 0x38,2,8,JR_CC_N(readC, (char)r8)			 ) \
x(0x4, 0x0, 0x39,1,8,ADD_HL_N(CPU::Registers.SP)  						) \
x(0x0, 0x0, 0x3A,1,8,LDD_R_D(A, HL_mv)               					) \
x(0x0, 0x0, 0x3B,1,8,Registers.SP -= 0x1;       									) \
x(0x4, 0x0, 0x3C,1,4,INC_R(A)        									) \
x(0x0, 0x4, 0x3D,1,4,DEC_R(A)		   									) \
x(0x0, 0x0, 0x3E,2,8,LD_R_D(A, d8)   									) \
x(0x6, 0x0, 0x3F,1,4,Registers.F ^= 0x10 								) \
x(0x0, 0x0, 0x40,1,4,LD_R_R(B, B)    									) \
x(0x0, 0x0, 0x41,1,4,LD_R_R(B, C)    									) \
x(0x0, 0x0, 0x42,1,4,LD_R_R(B, D)    									) \
x(0x0, 0x0, 0x43,1,4,LD_R_R(B, E)    									) \
x(0x0, 0x0, 0x44,1,4,LD_R_R(B, H)    									) \
x(0x0, 0x0, 0x45,1,4,LD_R_R(B, L)    									) \
x(0x0, 0x0, 0x46,1,8,LD_R_D(B, HL_mv) 									) \
x(0x0, 0x0, 0x47,1,4,LD_R_R(B, A)    									) \
x(0x0, 0x0, 0x48,1,4,LD_R_R(C, B)    									) \
x(0x0, 0x0, 0x49,1,4,LD_R_R(C, C)    									) \
x(0x0, 0x0, 0x4A,1,4,LD_R_R(C, D)    									) \
x(0x0, 0x0, 0x4B,1,4,LD_R_R(C, E)    									) \
x(0x0, 0x0, 0x4C,1,4,LD_R_R(C, H)    									) \
x(0x0, 0x0, 0x4D,1,4,LD_R_R(C, L)    									) \
x(0x0, 0x0, 0x4E,1,8,LD_R_D(C, HL_mv) 									) \
x(0x0, 0x0, 0x4F,1,4,LD_R_R(C, A)    									) \
x(0x0, 0x0, 0x50,1,4,LD_R_R(D, B)    									) \
x(0x0, 0x0, 0x51,1,4,LD_R_R(D, C)    									) \
x(0x0, 0x0, 0x52,1,4,LD_R_R(D, D)    									) \
x(0x0, 0x0, 0x53,1,4,LD_R_R(D, E)    									) \
x(0x0, 0x0, 0x54,1,4,LD_R_R(D, H)    									) \
x(0x0, 0x0, 0x55,1,4,LD_R_R(D, L)    									) \
x(0x0, 0x0, 0x56,1,8,LD_R_D(D, HL_mv) 									) \
x(0x0, 0x0, 0x57,1,4,LD_R_R(D, A)    									) \
x(0x0, 0x0, 0x58,1,4,LD_R_R(E, B)    									) \
x(0x0, 0x0, 0x59,1,4,LD_R_R(E, C)    									) \
x(0x0, 0x0, 0x5A,1,4,LD_R_R(E, D)    									) \
x(0x0, 0x0, 0x5B,1,4,LD_R_R(E, E)    									) \
x(0x0, 0x0, 0x5C,1,4,LD_R_R(E, H)    									) \
x(0x0, 0x0, 0x5D,1,4,LD_R_R(E, L)    									) \
x(0x0, 0x0, 0x5E,1,8,LD_R_D(E, HL_mv) 									) \
x(0x0, 0x0, 0x5F,1,4,LD_R_R(E, A)    									) \
x(0x0, 0x0, 0x60,1,4,LD_R_R(H, B)    									) \
x(0x0, 0x0, 0x61,1,4,LD_R_R(H, C)    									) \
x(0x0, 0x0, 0x62,1,4,LD_R_R(H, D)    									) \
x(0x0, 0x0, 0x63,1,4,LD_R_R(H, E)    									) \
x(0x0, 0x0, 0x64,1,4,LD_R_R(H, H)    									) \
x(0x0, 0x0, 0x65,1,4,LD_R_R(H, L)    									) \
x(0x0, 0x0, 0x66,1,8,LD_R_D(H, HL_mv) 									) \
x(0x0, 0x0, 0x67,1,4,LD_R_R(H, A)    									) \
x(0x0, 0x0, 0x68,1,4,LD_R_R(L, B)    									) \
x(0x0, 0x0, 0x69,1,4,LD_R_R(L, C)    									) \
x(0x0, 0x0, 0x6A,1,4,LD_R_R(L, D)    									) \
x(0x0, 0x0, 0x6B,1,4,LD_R_R(L, E)    									) \
x(0x0, 0x0, 0x6C,1,4,LD_R_R(L, H)    									) \
x(0x0, 0x0, 0x6D,1,4,LD_R_R(L, L)    									) \
x(0x0, 0x0, 0x6E,1,8,LD_R_D(L, HL_mv) 									) \
x(0x0, 0x0, 0x6F,1,4,LD_R_R(L, A)    									) \
x(0x0, 0x0, 0x70,1,8,LD_D_R(HL_v, B) 									) \
x(0x0, 0x0, 0x71,1,8,LD_D_R(HL_v, C) 									) \
x(0x0, 0x0, 0x72,1,8,LD_D_R(HL_v, D) 									) \
x(0x0, 0x0, 0x73,1,8,LD_D_R(HL_v, E) 									) \
x(0x0, 0x0, 0x74,1,8,LD_D_R(HL_v, H) 									) \
x(0x0, 0x0, 0x75,1,8,LD_D_R(HL_v, L) 									) \
x(0x0, 0x0, 0x76,1,4,cpuHalt = true;									) \
x(0x0, 0x0, 0x77,1,8,LD_D_R(HL_v, A) 									) \
x(0x0, 0x0, 0x78,1,4,LD_R_R(A, B)    									) \
x(0x0, 0x0, 0x79,1,4,LD_R_R(A, C)    									) \
x(0x0, 0x0, 0x7A,1,4,LD_R_R(A, D)    									) \
x(0x0, 0x0, 0x7B,1,4,LD_R_R(A, E)    									) \
x(0x0, 0x0, 0x7C,1,4,LD_R_R(A, H)    									) \
x(0x0, 0x0, 0x7D,1,4,LD_R_R(A, L)    									) \
x(0x0, 0x0, 0x7E,1,8,LD_R_D(A, HL_mv) 									) \
x(0x0, 0x0, 0x7F,1,4,LD_R_R(A, A)    									) \
x(0x4, 0x0, 0x80,1,4,ADD_R_R(A, B)   									) \
x(0x4, 0x0, 0x81,1,4,ADD_R_R(A, C)   									) \
x(0x4, 0x0, 0x82,1,4,ADD_R_R(A, D)   									) \
x(0x4, 0x0, 0x83,1,4,ADD_R_R(A, E)   									) \
x(0x4, 0x0, 0x84,1,4,ADD_R_R(A, H)   									) \
x(0x4, 0x0, 0x85,1,4,ADD_R_R(A, L)   									) \
x(0x4, 0x0, 0x86,1,8,ADD_R_D(A, HL_mv)									) \
x(0x4, 0x0, 0x87,1,4,ADD_R_R(A, A)   									) \
x(0x4, 0x0, 0x88,1,4,ADC_R(B)        									) \
x(0x4, 0x0, 0x89,1,4,ADC_R(C)		   									) \
x(0x4, 0x0, 0x8A,1,4,ADC_R(D)		   									) \
x(0x4, 0x0, 0x8B,1,4,ADC_R(E)		   									) \
x(0x4, 0x0, 0x8C,1,4,ADC_R(H)		   									) \
x(0x4, 0x0, 0x8D,1,4,ADC_R(L)		   									) \
x(0x4, 0x0, 0x8E,1,8,ADC_D(HL_mv)     									) \
x(0x4, 0x0, 0x8F,1,4,ADC_R(A)        									) \
x(0x0, 0x4, 0x90,1,4,SUB_R_R(A, B)   									) \
x(0x0, 0x4, 0x91,1,4,SUB_R_R(A, C)   									) \
x(0x0, 0x4, 0x92,1,4,SUB_R_R(A, D)   									) \
x(0x0, 0x4, 0x93,1,4,SUB_R_R(A, E)   									) \
x(0x0, 0x4, 0x94,1,4,SUB_R_R(A, H)   									) \
x(0x0, 0x4, 0x95,1,4,SUB_R_R(A, L)   									) \
x(0x0, 0x4, 0x96,1,8,SUB_R_D(A, HL_mv)									) \
x(0x0, 0x4, 0x97,1,4,SUB_R_R(A, A)   									) \
x(0x0, 0x4, 0x98,1,4,SBC_R(B)   	   									) \
x(0x0, 0x4, 0x99,1,4,SBC_R(C)		   									) \
x(0x0, 0x4, 0x9A,1,4,SBC_R(D)		   									) \
x(0x0, 0x4, 0x9B,1,4,SBC_R(E)		   									) \
x(0x0, 0x4, 0x9C,1,4,SBC_R(H)		   									) \
x(0x0, 0x4, 0x9D,1,4,SBC_R(L)		   									) \
x(0x0, 0x4, 0x9E,1,8,SBC_D(HL_mv)	   									) \
x(0x0, 0x4, 0x9F,1,4,SBC_R(A)   	   									) \
x(0x5, 0x2, 0xA0,1,4,AND_R(B)        									) \
x(0x5, 0x2, 0xA1,1,4,AND_R(C)        									) \
x(0x5, 0x2, 0xA2,1,4,AND_R(D)        									) \
x(0x5, 0x2, 0xA3,1,4,AND_R(E)        									) \
x(0x5, 0x2, 0xA4,1,4,AND_R(H)        									) \
x(0x5, 0x2, 0xA5,1,4,AND_R(L)        									) \
x(0x5, 0x2, 0xA6,1,8,AND_D(HL_mv)     									) \
x(0x5, 0x2, 0xA7,1,4,AND_R(A)        									) \
x(0x7, 0x0, 0xA8,1,4,XOR_R(B)        									) \
x(0x7, 0x0, 0xA9,1,4,XOR_R(C)        									) \
x(0x7, 0x0, 0xAA,1,4,XOR_R(D)        									) \
x(0x7, 0x0, 0xAB,1,4,XOR_R(E)        									) \
x(0x7, 0x0, 0xAC,1,4,XOR_R(H)        									) \
x(0x7, 0x0, 0xAD,1,4,XOR_R(L)        									) \
x(0x7, 0x0, 0xAE,1,8,XOR_D(HL_mv)     									) \
x(0x7, 0x0, 0xAF,1,4,XOR_R(A)        									) \
x(0x7, 0x0, 0xB0,1,4,OR_R(B)        									) \
x(0x7, 0x0, 0xB1,1,4,OR_R(C)        									) \
x(0x7, 0x0, 0xB2,1,4,OR_R(D)        									) \
x(0x7, 0x0, 0xB3,1,4,OR_R(E)        									) \
x(0x7, 0x0, 0xB4,1,4,OR_R(H)        									) \
x(0x7, 0x0, 0xB5,1,4,OR_R(L)        									) \
x(0x7, 0x0, 0xB6,1,8,OR_D(HL_mv)     									) \
x(0x7, 0x0, 0xB7,1,4,OR_R(A)        									) \
x(0x0, 0x4, 0xB8,1,4,CP_R(B)                							) \
x(0x0, 0x4, 0xB9,1,4,CP_R(C)                							) \
x(0x0, 0x4, 0xBA,1,4,CP_R(D)                							) \
x(0x0, 0x4, 0xBB,1,4,CP_R(E)                							) \
x(0x0, 0x4, 0xBC,1,4,CP_R(H)                							) \
x(0x0, 0x4, 0xBD,1,4,CP_R(L)                							) \
x(0x0, 0x4, 0xBE,1,8,CP_D(HL_mv)                						) \
x(0x0, 0x4, 0xBF,1,4,CP_R(A)                							) \
x(0x0, 0x0, 0xC0,1,8,RET_XX(!readZ)               						) \
x(0x0, 0x0, 0xC1,1,12,POP_NN(BC)              							) \
x(0x0, 0x0, 0xC2,3,12,JP_CC_N(!readZ,a16)               				) \
x(0x0, 0x0, 0xC3,3,16,JP_N(a16)											) \
x(0x0, 0x0, 0xC4,3,12,CALL_XX_NN(!readZ, a16)               			) \
x(0x0, 0x0, 0xC5,1,16,PUSH_NN(readBC())               					) \
x(0x4, 0x0, 0xC6,2,8,ADD_R_D(A, d8)  									) \
x(0x0, 0x0, 0xC7,1,16,RST_N(0x0)              							) \
x(0x0, 0x0, 0xC8,1,8, RET_XX(readZ)               						) \
x(0x0, 0x0, 0xC9,1,16,RET()               								) \
x(0x0, 0x0, 0xCA,3,12,JP_CC_N(readZ, a16)               				) \
x(0x0, 0x0, 0xCC,3,12,CALL_XX_NN(readZ, a16)               				) \
x(0x0, 0x0, 0xCD,3,24,CALL_NN(a16)               						) \
x(0x4, 0x0, 0xCE,2,8,ADC_D(d8)              							) \
x(0x0, 0x0, 0xCF,1,16,RST_N(0x08)               						) \
x(0x0, 0x0, 0xD0,1,8, RET_XX(!readC)               						) \
x(0x0, 0x0, 0xD1,1,12,POP_NN(DE)               							) \
x(0x0, 0x0, 0xD2,3,12,JP_CC_N(!readC,a16)               				) \
x(0x0, 0x0, 0xD4,3,12,CALL_XX_NN(!readC, a16)               			) \
x(0x0, 0x0, 0xD5,1,16,PUSH_NN(readDE())               					) \
x(0x0, 0x4, 0xD6,2,8,SUB_R_D(A, d8)               						) \
x(0x0, 0x0, 0xD7,1,16,RST_N(0x10)               						) \
x(0x0, 0x0, 0xD8,1,8,RET_XX(readC)                						) \
x(0x0, 0x0, 0xD9,1,16,RETI()              								) \
x(0x0, 0x0, 0xDA,3,12,JP_CC_N(readC, a16)               				) \
x(0x0, 0x0, 0xDC,3,12,CALL_XX_NN(readC, a16)               				) \
x(0x0, 0x4, 0xDE,2,8, SBC_D(d8)               							) \
x(0x0, 0x0, 0xDF,1,16,RST_N(0x18)               						) \
x(0x0, 0x0, 0xE0,2,12,LD_D_R(0xFF00 + a8, A)               				) \
x(0x0, 0x0, 0xE1,1,12,POP_NN(HL)               							) \
x(0x0, 0x0, 0xE2,1,8,LD_D_R(0xFF00 + CPU::Registers.C, A)    			) \
x(0x0, 0x0, 0xE5,1,16,PUSH_NN(readHL())               					) \
x(0x5, 0x2, 0xE6,2,8,AND_D(d8)                							) \
x(0x0, 0x0, 0xE7,1,16,RST_N(0x20)                						) \
x(0xC, 0x0, 0xE8,2,16,ADD_SP_R8()								) \
x(0x0, 0x0, 0xE9,1,4,CPU::Registers.PC = readHL(); preservePC = true; 		) \
x(0x0, 0x0, 0xEA,3,16, LD_D_R(a16, A)				) \
x(0x7, 0x0, 0xEE,2,8, XOR_D(d8)              							) \
x(0x0, 0x0, 0xEF,1,16,RST_N(0x28)              							) \
x(0x0, 0x0, 0xF0,2,12,LD_R_D(A, RAM::read(0xFF00 + a8))               	) \
x(0x0, 0x0, 0xF1,1,12,POP_NN(AF) 									) \
x(0x0, 0x0, 0xF2,2,8,LD_R_D(A, RAM::read(0xFF00 + CPU::Registers.C) ) 	) \
x(0x0, 0x0, 0xF3,1,4, DI()           								) \
x(0x0, 0x0, 0xF5,1,16, PUSH_NN(readAF())             						) \
x(0x7, 0x0, 0xF6,2,8, OR_D(d8)             								) \
x(0x0, 0x0, 0xF7,1,16,RST_N(0x30)              							) \
x(0xC, 0x0, 0xF8,2,12,LD_HL_SPR8()										) \
x(0x0, 0x0, 0xF9,1,8,Registers.SP = readHL() 							) \
x(0x0, 0x0, 0xFA,3,16, LD_R_D(A, RAM::read(a16))       					) \
x(0x0, 0x0, 0xFB,1,4, EI()             							) \
x(0x0, 0x4, 0xFE,2,8,CP_D(d8)                							) \
x(0x0, 0x0, 0xFF,1,16,RST_N(0x38)               						)

//  End of INSTRUCTIONS()

#define INSTRUCTIONS_CB(y) \
y(0x6, 0x0, 0x00,2,8, RLC_R(B)              ) \
y(0x6, 0x0, 0x01,2,8, RLC_R(C)              ) \
y(0x6, 0x0, 0x02,2,8, RLC_R(D)              ) \
y(0x6, 0x0, 0x03,2,8, RLC_R(E)              ) \
y(0x6, 0x0, 0x04,2,8, RLC_R(H)              ) \
y(0x6, 0x0, 0x05,2,8, RLC_R(L)              ) \
y(0x6, 0x0, 0x06,2,15,RLC_D(HL_v)           ) \
y(0x6, 0x0, 0x07,2,8, RLC_R(A)              ) \
y(0x6, 0x0, 0x08,2,8, RRC_R(B)              ) \
y(0x6, 0x0, 0x09,2,8, RRC_R(C)              ) \
y(0x6, 0x0, 0x0A,2,8, RRC_R(D)              ) \
y(0x6, 0x0, 0x0B,2,8, RRC_R(E)              ) \
y(0x6, 0x0, 0x0C,2,8, RRC_R(H)              ) \
y(0x6, 0x0, 0x0D,2,8, RRC_R(L)              ) \
y(0x6, 0x0, 0x0E,2,15,RRC_D(HL_v)           ) \
y(0x6, 0x0, 0x0F,2,8, RRC_R(A)              ) \
y(0x6, 0x0, 0x10,2,8, RL_R(B)               ) \
y(0x6, 0x0, 0x11,2,8, RL_R(C)               ) \
y(0x6, 0x0, 0x12,2,8, RL_R(D)               ) \
y(0x6, 0x0, 0x13,2,8, RL_R(E)               ) \
y(0x6, 0x0, 0x14,2,8, RL_R(H)               ) \
y(0x6, 0x0, 0x15,2,8, RL_R(L)               ) \
y(0x6, 0x0, 0x16,2,15,RL_D(HL_v)            ) \
y(0x6, 0x0, 0x17,2,8, RL_R(A)               ) \
y(0x6, 0x0, 0x18,2,8, RR_R(B)               ) \
y(0x6, 0x0, 0x19,2,8, RR_R(C)               ) \
y(0x6, 0x0, 0x1A,2,8, RR_R(D)               ) \
y(0x6, 0x0, 0x1B,2,8, RR_R(E)               ) \
y(0x6, 0x0, 0x1C,2,8, RR_R(H)               ) \
y(0x6, 0x0, 0x1D,2,8, RR_R(L)               ) \
y(0x6, 0x0, 0x1E,2,15,RR_D(HL_v)            ) \
y(0x6, 0x0, 0x1F,2,8, RR_R(A)               ) \
y(0x6, 0x0, 0x20,2,8, SLA_R(B)              ) \
y(0x6, 0x0, 0x21,2,8, SLA_R(C)              ) \
y(0x6, 0x0, 0x22,2,8, SLA_R(D)              ) \
y(0x6, 0x0, 0x23,2,8, SLA_R(E)              ) \
y(0x6, 0x0, 0x24,2,8, SLA_R(H)              ) \
y(0x6, 0x0, 0x25,2,8, SLA_R(L)              ) \
y(0x6, 0x0, 0x26,2,15,SLA_D(HL_v)           ) \
y(0x6, 0x0, 0x27,2,8, SLA_R(A)              ) \
y(0x6, 0x0, 0x28,2,8, SRA_R(B)              ) \
y(0x6, 0x0, 0x29,2,8, SRA_R(C)              ) \
y(0x6, 0x0, 0x2A,2,8, SRA_R(D)              ) \
y(0x6, 0x0, 0x2B,2,8, SRA_R(E)              ) \
y(0x6, 0x0, 0x2C,2,8, SRA_R(H)              ) \
y(0x6, 0x0, 0x2D,2,8, SRA_R(L)              ) \
y(0x6, 0x0, 0x2E,2,15,SRA_D(HL_v)           ) \
y(0x6, 0x0, 0x2F,2,8, SRA_R(A)              ) \
y(0x7, 0x0, 0x30,2,8, SWAP_R(B)             ) \
y(0x7, 0x0, 0x31,2,8, SWAP_R(C)             ) \
y(0x7, 0x0, 0x32,2,8, SWAP_R(D)             ) \
y(0x7, 0x0, 0x33,2,8, SWAP_R(E)             ) \
y(0x7, 0x0, 0x34,2,8, SWAP_R(H)             ) \
y(0x7, 0x0, 0x35,2,8, SWAP_R(L)             ) \
y(0x7, 0x0, 0x36,2,15,SWAP_D(HL_v)          ) \
y(0x7, 0x0, 0x37,2,8, SWAP_R(A)             ) \
y(0x6, 0x0, 0x38,2,8, SRL_R(B)             ) \
y(0x6, 0x0, 0x39,2,8, SRL_R(C)             ) \
y(0x6, 0x0, 0x3A,2,8, SRL_R(D)             ) \
y(0x6, 0x0, 0x3B,2,8, SRL_R(E)             ) \
y(0x6, 0x0, 0x3C,2,8, SRL_R(H)             ) \
y(0x6, 0x0, 0x3D,2,8, SRL_R(L)             ) \
y(0x6, 0x0, 0x3E,2,15,SRL_D(HL_v)          ) \
y(0x6, 0x0, 0x3F,2,8, SRL_R(A)             ) \
y(0x4, 0x2, 0x40,2,8, BITN_R(0, B)          ) \
y(0x4, 0x2, 0x41,2,8, BITN_R(0, C)          ) \
y(0x4, 0x2, 0x42,2,8, BITN_R(0, D)          ) \
y(0x4, 0x2, 0x43,2,8, BITN_R(0, E)          ) \
y(0x4, 0x2, 0x44,2,8, BITN_R(0, H)          ) \
y(0x4, 0x2, 0x45,2,8, BITN_R(0, L)          ) \
y(0x4, 0x2, 0x46,2,12,BITN_D(0, HL_mv)      ) \
y(0x4, 0x2, 0x47,2,8, BITN_R(0, A)          ) \
y(0x4, 0x2, 0x48,2,8, BITN_R(1, B)          ) \
y(0x4, 0x2, 0x49,2,8, BITN_R(1, C)          ) \
y(0x4, 0x2, 0x4A,2,8, BITN_R(1, D)          ) \
y(0x4, 0x2, 0x4B,2,8, BITN_R(1, E)          ) \
y(0x4, 0x2, 0x4C,2,8, BITN_R(1, H)          ) \
y(0x4, 0x2, 0x4D,2,8, BITN_R(1, L)          ) \
y(0x4, 0x2, 0x4E,2,12,BITN_D(1, HL_mv)      ) \
y(0x4, 0x2, 0x4F,2,8, BITN_R(1, A)          ) \
y(0x4, 0x2, 0x50,2,8, BITN_R(2, B)          ) \
y(0x4, 0x2, 0x51,2,8, BITN_R(2, C)          ) \
y(0x4, 0x2, 0x52,2,8, BITN_R(2, D)          ) \
y(0x4, 0x2, 0x53,2,8, BITN_R(2, E)          ) \
y(0x4, 0x2, 0x54,2,8, BITN_R(2, H)          ) \
y(0x4, 0x2, 0x55,2,8, BITN_R(2, L)          ) \
y(0x4, 0x2, 0x56,2,12,BITN_D(2, HL_mv)      ) \
y(0x4, 0x2, 0x57,2,8, BITN_R(2, A)          ) \
y(0x4, 0x2, 0x58,2,8, BITN_R(3, B)          ) \
y(0x4, 0x2, 0x59,2,8, BITN_R(3, C)          ) \
y(0x4, 0x2, 0x5A,2,8, BITN_R(3, D)          ) \
y(0x4, 0x2, 0x5B,2,8, BITN_R(3, E)          ) \
y(0x4, 0x2, 0x5C,2,8, BITN_R(3, H)          ) \
y(0x4, 0x2, 0x5D,2,8, BITN_R(3, L)          ) \
y(0x4, 0x2, 0x5E,2,12,BITN_D(3, HL_mv)      ) \
y(0x4, 0x2, 0x5F,2,8, BITN_R(3, A)          ) \
y(0x4, 0x2, 0x60,2,8, BITN_R(4, B)          ) \
y(0x4, 0x2, 0x61,2,8, BITN_R(4, C)          ) \
y(0x4, 0x2, 0x62,2,8, BITN_R(4, D)          ) \
y(0x4, 0x2, 0x63,2,8, BITN_R(4, E)          ) \
y(0x4, 0x2, 0x64,2,8, BITN_R(4, H)          ) \
y(0x4, 0x2, 0x65,2,8, BITN_R(4, L)          ) \
y(0x4, 0x2, 0x66,2,12,BITN_D(4, HL_mv)      ) \
y(0x4, 0x2, 0x67,2,8, BITN_R(4, A)          ) \
y(0x4, 0x2, 0x68,2,8, BITN_R(5, B)          ) \
y(0x4, 0x2, 0x69,2,8, BITN_R(5, C)          ) \
y(0x4, 0x2, 0x6A,2,8, BITN_R(5, D)          ) \
y(0x4, 0x2, 0x6B,2,8, BITN_R(5, E)          ) \
y(0x4, 0x2, 0x6C,2,8, BITN_R(5, H)          ) \
y(0x4, 0x2, 0x6D,2,8, BITN_R(5, L)          ) \
y(0x4, 0x2, 0x6E,2,12,BITN_D(5, HL_mv)      ) \
y(0x4, 0x2, 0x6F,2,8, BITN_R(5, A)          ) \
y(0x4, 0x2, 0x70,2,8, BITN_R(6, B)          ) \
y(0x4, 0x2, 0x71,2,8, BITN_R(6, C)          ) \
y(0x4, 0x2, 0x72,2,8, BITN_R(6, D)          ) \
y(0x4, 0x2, 0x73,2,8, BITN_R(6, E)          ) \
y(0x4, 0x2, 0x74,2,8, BITN_R(6, H)          ) \
y(0x4, 0x2, 0x75,2,8, BITN_R(6, L)          ) \
y(0x4, 0x2, 0x76,2,12,BITN_D(6, HL_mv)      ) \
y(0x4, 0x2, 0x77,2,8, BITN_R(6, A)          ) \
y(0x4, 0x2, 0x78,2,8, BITN_R(7, B)          ) \
y(0x4, 0x2, 0x79,2,8, BITN_R(7, C)          ) \
y(0x4, 0x2, 0x7A,2,8, BITN_R(7, D)          ) \
y(0x4, 0x2, 0x7B,2,8, BITN_R(7, E)          ) \
y(0x4, 0x2, 0x7C,2,8, BITN_R(7, H)          ) \
y(0x4, 0x2, 0x7D,2,8, BITN_R(7, L)          ) \
y(0x4, 0x2, 0x7E,2,12,BITN_D(7, HL_mv)      ) \
y(0x4, 0x2, 0x7F,2,8, BITN_R(7, A)          ) \
y(0x0, 0x0, 0x80,2,8, resetBitN_R(0, B)     ) \
y(0x0, 0x0, 0x81,2,8, resetBitN_R(0, C)     ) \
y(0x0, 0x0, 0x82,2,8, resetBitN_R(0, D)     ) \
y(0x0, 0x0, 0x83,2,8, resetBitN_R(0, E)     ) \
y(0x0, 0x0, 0x84,2,8, resetBitN_R(0, H)     ) \
y(0x0, 0x0, 0x85,2,8, resetBitN_R(0, L)     ) \
y(0x0, 0x0, 0x86,2,15,resetBitN_D(0, HL_v)  ) \
y(0x0, 0x0, 0x87,2,8, resetBitN_R(0, A)     ) \
y(0x0, 0x0, 0x88,2,8, resetBitN_R(1, B)     ) \
y(0x0, 0x0, 0x89,2,8, resetBitN_R(1, C)     ) \
y(0x0, 0x0, 0x8A,2,8, resetBitN_R(1, D)     ) \
y(0x0, 0x0, 0x8B,2,8, resetBitN_R(1, E)     ) \
y(0x0, 0x0, 0x8C,2,8, resetBitN_R(1, H)     ) \
y(0x0, 0x0, 0x8D,2,8, resetBitN_R(1, L)     ) \
y(0x0, 0x0, 0x8E,2,15,resetBitN_D(1, HL_v)  ) \
y(0x0, 0x0, 0x8F,2,8, resetBitN_R(1, A)     ) \
y(0x0, 0x0, 0x90,2,8, resetBitN_R(2, B)     ) \
y(0x0, 0x0, 0x91,2,8, resetBitN_R(2, C)     ) \
y(0x0, 0x0, 0x92,2,8, resetBitN_R(2, D)     ) \
y(0x0, 0x0, 0x93,2,8, resetBitN_R(2, E)     ) \
y(0x0, 0x0, 0x94,2,8, resetBitN_R(2, H)     ) \
y(0x0, 0x0, 0x95,2,8, resetBitN_R(2, L)     ) \
y(0x0, 0x0, 0x96,2,15,resetBitN_D(2, HL_v)  ) \
y(0x0, 0x0, 0x97,2,8, resetBitN_R(2, A)     ) \
y(0x0, 0x0, 0x98,2,8, resetBitN_R(3, B)     ) \
y(0x0, 0x0, 0x99,2,8, resetBitN_R(3, C)     ) \
y(0x0, 0x0, 0x9A,2,8, resetBitN_R(3, D)     ) \
y(0x0, 0x0, 0x9B,2,8, resetBitN_R(3, E)     ) \
y(0x0, 0x0, 0x9C,2,8, resetBitN_R(3, H)     ) \
y(0x0, 0x0, 0x9D,2,8, resetBitN_R(3, L)     ) \
y(0x0, 0x0, 0x9E,2,15,resetBitN_D(3, HL_v)  ) \
y(0x0, 0x0, 0x9F,2,8, resetBitN_R(3, A)     ) \
y(0x0, 0x0, 0xA0,2,8, resetBitN_R(4, B)     ) \
y(0x0, 0x0, 0xA1,2,8, resetBitN_R(4, C)     ) \
y(0x0, 0x0, 0xA2,2,8, resetBitN_R(4, D)     ) \
y(0x0, 0x0, 0xA3,2,8, resetBitN_R(4, E)     ) \
y(0x0, 0x0, 0xA4,2,8, resetBitN_R(4, H)     ) \
y(0x0, 0x0, 0xA5,2,8, resetBitN_R(4, L)     ) \
y(0x0, 0x0, 0xA6,2,15,resetBitN_D(4, HL_v)  ) \
y(0x0, 0x0, 0xA7,2,8, resetBitN_R(4, A)     ) \
y(0x0, 0x0, 0xA8,2,8, resetBitN_R(5, B)     ) \
y(0x0, 0x0, 0xA9,2,8, resetBitN_R(5, C)     ) \
y(0x0, 0x0, 0xAA,2,8, resetBitN_R(5, D)     ) \
y(0x0, 0x0, 0xAB,2,8, resetBitN_R(5, E)     ) \
y(0x0, 0x0, 0xAC,2,8, resetBitN_R(5, H)     ) \
y(0x0, 0x0, 0xAD,2,8, resetBitN_R(5, L)     ) \
y(0x0, 0x0, 0xAE,2,15,resetBitN_D(5, HL_v)  ) \
y(0x0, 0x0, 0xAF,2,8, resetBitN_R(5, A)     ) \
y(0x0, 0x0, 0xB0,2,8, resetBitN_R(6, B)     ) \
y(0x0, 0x0, 0xB1,2,8, resetBitN_R(6, C)     ) \
y(0x0, 0x0, 0xB2,2,8, resetBitN_R(6, D)     ) \
y(0x0, 0x0, 0xB3,2,8, resetBitN_R(6, E)     ) \
y(0x0, 0x0, 0xB4,2,8, resetBitN_R(6, H)     ) \
y(0x0, 0x0, 0xB5,2,8, resetBitN_R(6, L)     ) \
y(0x0, 0x0, 0xB6,2,15,resetBitN_D(6, HL_v)  ) \
y(0x0, 0x0, 0xB7,2,8, resetBitN_R(6, A)     ) \
y(0x0, 0x0, 0xB8,2,8, resetBitN_R(7, B)     ) \
y(0x0, 0x0, 0xB9,2,8, resetBitN_R(7, C)     ) \
y(0x0, 0x0, 0xBA,2,8, resetBitN_R(7, D)     ) \
y(0x0, 0x0, 0xBB,2,8, resetBitN_R(7, E)     ) \
y(0x0, 0x0, 0xBC,2,8, resetBitN_R(7, H)     ) \
y(0x0, 0x0, 0xBD,2,8, resetBitN_R(7, L)     ) \
y(0x0, 0x0, 0xBE,2,15,resetBitN_D(7, HL_v)  ) \
y(0x0, 0x0, 0xBF,2,8, resetBitN_R(7, A)     ) \
y(0x0, 0x0, 0xC0,2,8, setBitN_R(0,B)  		) \
y(0x0, 0x0, 0xC1,2,8, setBitN_R(0,C)        ) \
y(0x0, 0x0, 0xC2,2,8, setBitN_R(0,D)        ) \
y(0x0, 0x0, 0xC3,2,8, setBitN_R(0,E)        ) \
y(0x0, 0x0, 0xC4,2,8, setBitN_R(0,H)        ) \
y(0x0, 0x0, 0xC5,2,8, setBitN_R(0,L)        ) \
y(0x0, 0x0, 0xC6,2,15,setBitN_D(0,HL_v)     ) \
y(0x0, 0x0, 0xC7,2,8, setBitN_R(0,A)		) \
y(0x0, 0x0, 0xC8,2,8, setBitN_R(1,B)		) \
y(0x0, 0x0, 0xC9,2,8, setBitN_R(1,C)		) \
y(0x0, 0x0, 0xCA,2,8, setBitN_R(1,D)		) \
y(0x0, 0x0, 0xCB,2,8, setBitN_R(1,E)		) \
y(0x0, 0x0, 0xCC,2,8, setBitN_R(1,H)		) \
y(0x0, 0x0, 0xCD,2,8, setBitN_R(1,L)		) \
y(0x0, 0x0, 0xCE,2,15,setBitN_D(1,HL_v)		) \
y(0x0, 0x0, 0xCF,2,8, setBitN_R(1,A)		) \
y(0x0, 0x0, 0xD0,2,8, setBitN_R(2,B)		) \
y(0x0, 0x0, 0xD1,2,8, setBitN_R(2,C)		) \
y(0x0, 0x0, 0xD2,2,8, setBitN_R(2,D)		) \
y(0x0, 0x0, 0xD3,2,8, setBitN_R(2,E)		) \
y(0x0, 0x0, 0xD4,2,8, setBitN_R(2,H)		) \
y(0x0, 0x0, 0xD5,2,8, setBitN_R(2,L)		) \
y(0x0, 0x0, 0xD6,2,15,setBitN_D(2,HL_v)		) \
y(0x0, 0x0, 0xD7,2,8, setBitN_R(2,A)		) \
y(0x0, 0x0, 0xD8,2,8, setBitN_R(3,B)		) \
y(0x0, 0x0, 0xD9,2,8, setBitN_R(3,C)		) \
y(0x0, 0x0, 0xDA,2,8, setBitN_R(3,D)		) \
y(0x0, 0x0, 0xDB,2,8, setBitN_R(3,E)		) \
y(0x0, 0x0, 0xDC,2,8, setBitN_R(3,H)		) \
y(0x0, 0x0, 0xDD,2,8, setBitN_R(3,L)		) \
y(0x0, 0x0, 0xDE,2,15,setBitN_D(3,HL_v)		) \
y(0x0, 0x0, 0xDF,2,8, setBitN_R(3,A)		) \
y(0x0, 0x0, 0xE0,2,8, setBitN_R(4,B)		) \
y(0x0, 0x0, 0xE1,2,8, setBitN_R(4,C)		) \
y(0x0, 0x0, 0xE2,2,8, setBitN_R(4,D)		) \
y(0x0, 0x0, 0xE3,2,8, setBitN_R(4,E)		) \
y(0x0, 0x0, 0xE4,2,8, setBitN_R(4,H)		) \
y(0x0, 0x0, 0xE5,2,8, setBitN_R(4,L)		) \
y(0x0, 0x0, 0xE6,2,15,setBitN_D(4,HL_v)		) \
y(0x0, 0x0, 0xE7,2,8, setBitN_R(4,A)		) \
y(0x0, 0x0, 0xE8,2,8, setBitN_R(5,B)		) \
y(0x0, 0x0, 0xE9,2,8, setBitN_R(5,C)		) \
y(0x0, 0x0, 0xEA,2,8, setBitN_R(5,D)		) \
y(0x0, 0x0, 0xEB,2,8, setBitN_R(5,E)		) \
y(0x0, 0x0, 0xEC,2,8, setBitN_R(5,H)		) \
y(0x0, 0x0, 0xED,2,8, setBitN_R(5,L)		) \
y(0x0, 0x0, 0xEE,2,15,setBitN_D(5,HL_v)		) \
y(0x0, 0x0, 0xEF,2,8, setBitN_R(5,A)		) \
y(0x0, 0x0, 0xF0,2,8, setBitN_R(6,B)		) \
y(0x0, 0x0, 0xF1,2,8, setBitN_R(6,C)		) \
y(0x0, 0x0, 0xF2,2,8, setBitN_R(6,D)		) \
y(0x0, 0x0, 0xF3,2,8, setBitN_R(6,E)		) \
y(0x0, 0x0, 0xF4,2,8, setBitN_R(6,H)		) \
y(0x0, 0x0, 0xF5,2,8, setBitN_R(6,L)		) \
y(0x0, 0x0, 0xF6,2,15,setBitN_D(6,HL_v)		) \
y(0x0, 0x0, 0xF7,2,8, setBitN_R(6,A)		) \
y(0x0, 0x0, 0xF8,2,8, setBitN_R(7,B)		) \
y(0x0, 0x0, 0xF9,2,8, setBitN_R(7,C)		) \
y(0x0, 0x0, 0xFA,2,8, setBitN_R(7,D)		) \
y(0x0, 0x0, 0xFB,2,8, setBitN_R(7,E)		) \
y(0x0, 0x0, 0xFC,2,8, setBitN_R(7,H)		) \
y(0x0, 0x0, 0xFD,2,8, setBitN_R(7,L)		) \
y(0x0, 0x0, 0xFE,2,15,setBitN_D(7,HL_v)		) \
y(0x0, 0x0, 0xFF,2,8, setBitN_R(7,A)		)

//  End of INSTRUCTIONS_CB(y)

namespace CPU{
	Reg Registers;

	//  CPU Stuff
	bool emuHalt = false, step = true, continueExec = false, reload = false, /*shouldDisableInts = false,*/ shouldEnableInts = false, cpuHalt = false;
	char16_t prevPC;
	unsigned int freq = 4194304, freqDiv = 16384;
	double timerFreq;
	int64_t cycles = 0;
	int64_t timerCycles = 0, divCycles = 0;
	std::mutex mutex;
	/*
		Starts the emulator CPU
	*/
	void start(){
		timerFreq = freq / 1024.0;
		clearState();

		while(!emuHalt){
			if (reload) {
				Debug::emuLog("Reloading the emulator");
				RAM::clear();
				RAM::insert(RAM::bootRom, 0x0, RAM::bootRom.size(), 0, true);
				if (Config::getKeyState("SKIP_ROM") != "true") {
					RAM::insert(RAM::romFile, 0x100, 0x50, 0x100, false);
				}
				clearState();
				Debug::recentTraces.clear();
				Debug::menuText = "Reload completed";
				Debug::timer = 120;
				reload = false;
			}

			mutex.lock();
			if(step) cycle();
			mutex.unlock();

			timerCycles += cycles;
			divCycles += cycles;
			int timerEveryN = freq / (int)timerFreq;
			int divEveryN = freq / freqDiv;

			while(timerCycles > timerEveryN){
				if(RAM::RAM[0xFF07] & 0x4){
					if(RAM::RAM[0xFF05] + 1 > 0xFF){
						RAM::RAM[0xFF0F] |= 0x4;
						RAM::RAM[0xFF05] = RAM::RAM[0xFF06];
					} else {
						RAM::RAM[0xFF05] += 1;
					}
				}
				timerCycles -= timerEveryN;
			}

			while(divCycles > divEveryN){
				RAM::RAM[0xFF04] += 1;
				divCycles -= divEveryN;
			}

			PPU::handlePPU(cycles);




			if(emuHalt && Config::getKeyState("DEBUG_MODE") == "true"){
				Debug::menuText = "Emulation halted";
				Debug::timer = 240;
			}

			cycles = (CPU::cpuHalt) ? cycles : 0;
		}
	}


	/*
		Executes one CPU cycle/instruction
	*/
	bool cycle(){
		//  Interrupts
		//  Service and reset the IF flag
		if(!cpuHalt){
			if (CPU::Registers.IME && (RAM::read(0xFFFF) & 1) && (RAM::read(0xFF0F) & 1)) { // VBlank
				HandleInterrupt("VBlank", 0x40);
			} else if (CPU::Registers.IME && (RAM::read(0xFFFF) & 2) && (RAM::read(0xFF0F) & 2)) { // LCDC
				HandleInterrupt("LCDC", 0x48);
			} else if (CPU::Registers.IME && (RAM::read(0xFFFF) & 4) && (RAM::read(0xFF0F) & 4)) { // Timer
				HandleInterrupt("Timer", 0x50);
			} else if (CPU::Registers.IME && (RAM::read(0xFFFF) & 8) && (RAM::read(0xFF0F) & 8)) { // Serial
				HandleInterrupt("Serial", 0x58);
			} else if (CPU::Registers.IME && (RAM::read(0xFFFF) & 0x10) && (RAM::read(0xFF0F) & 0x10)) { // HiLo p10-p13
				HandleInterrupt("P10-13 HiLo", 0x60);
			}
		} else {
			if ( (RAM::read(0xFFFF) & 1) && (RAM::read(0xFF0F) & 1)) { // VBlank
				cpuHalt = false;
			} else if ( (RAM::read(0xFFFF) & 2) && (RAM::read(0xFF0F) & 2)) { // LCDC
				cpuHalt = false;
			} else if ( (RAM::read(0xFFFF) & 4) && (RAM::read(0xFF0F) & 4)) { // Timer
				cpuHalt = false;
			} else if ( (RAM::read(0xFFFF) & 8) && (RAM::read(0xFF0F) & 8)) { // Serial
				cpuHalt = false;
			} else if ( (RAM::read(0xFFFF) & 0x10) && (RAM::read(0xFF0F) & 0x10)) { // HiLo p10-p13
				cpuHalt = false;
			}
		}
		//  If in halt state
		if(cpuHalt){
			return true;
		}


		bool preservePC = false;
		unsigned char prefix = RAM::read(Registers.PC),
					  opcode = RAM::read(Registers.PC + 0x001),
					  d8 = RAM::read(Registers.PC + 0x001),
					  a8 = RAM::read(Registers.PC + 0x001);
		char16_t d16 = RAM::read(Registers.PC + 0x001) + RAM::read(Registers.PC + 0x002)*0x100,
			   	 a16 = d16;
		char     r8 = RAM::read(Registers.PC + 0x001);
		prevPC = Registers.PC;


		if (Config::getKeyState("DEBUG_MODE") == "true") {
			for (char16_t bp : Debug::breakpoints) {
				if (bp == Registers.PC) {
					step = false;
					continueExec = false;
					Debug::menuText = "Reached breakpoint at 0x" + Math::decHex(bp);
					Debug::timer = 180;
				}
			}
			for (char16_t bp : Debug::bpOp) {
				if (bp == prefix && prefix != 0xCB) {
					step = false;
					continueExec = false;
					Debug::menuText = "Instruction breakpoint $" + Math::decHex(bp);
					Debug::timer = 180;
				}
			}

			std::string trace = "PC: " + Math::decHex(Registers.PC) + "  AF:" + Math::decHex(Registers.A * 0x100 + Registers.F) + "  BC:" + Math::decHex(readBC()) + "  DE:" + Math::decHex(readDE()) + "  HL:" + Math::decHex(readHL()) + "  SP:" + Math::decHex(Registers.SP) + "  F:" + Math::decBin(Registers.F >> 4);
			if (Debug::recentTraces.size() > 20) {
				Debug::recentTraces.pop_front();
				Debug::recentTraces.emplace_back(trace);
			} else {
				Debug::recentTraces.emplace_back(trace);
			}
		}

		//  Turn off unused bits of register F
		Registers.F = Registers.F & 0xF0;

		if(prefix != 0xCB){
			//  Use normal op table
			switch(prefix){

				#define x(unsetMask,setMask,op,byte,cycle,ins) case op:{ ins; Registers.F |= (setMask << 4); Registers.F &= ~(unsetMask << 4); cycles += cycle; if(!preservePC){ Registers.PC += byte; }; break;}
				INSTRUCTIONS(x)
				#undef x

				default: Debug::emuLog("UNKNOWN OPCODE: " +  Math::decHex(prefix), Debug::LEVEL::ERR); break;
			}
		} else {
			// Use CB table
			switch(opcode){
			
				#define y(unsetMask,setMask,op,byte,cycle,ins) case op:{ ins; Registers.F |= (setMask << 4); Registers.F &= ~(unsetMask << 4); cycles += cycle; Registers.PC += byte; break; }
				INSTRUCTIONS_CB(y)
				#undef y

				default: Debug::emuLog("UNKNOWN CB OPCODE: " +  Math::decHex(opcode), Debug::LEVEL::ERR); break;
			}
		}

		if (opcode != 0xF3 && opcode != 0xFB && (shouldEnableInts)) {
			CPU::Registers.IME = true;
		}

		if (Config::getKeyState("DEBUG_MODE") == "true") step = false;
		if (continueExec) step = true;

		if(prevPC == Registers.PC) {
			//Debug::emuLog("Emulation Halted", Debug::LEVEL::INFO);
			//continueExec = false;
			//emuHalt = true;
		}
		return true;
	}

	/*
		Handles dispatching of interrupts
	*/
	void HandleInterrupt(std::string intName, unsigned char intVector){
		Debug::menuText = "int " + intName;
		Debug::timer = 240;
		CPU::Registers.IME = false;
		PUSH_NN(Registers.PC);
		Registers.PC = intVector;
		shouldEnableInts = false;
		RAM::RAM[0xFF0F] = 0xE0;
		cpuHalt = false;
	}

	/*
		Clears the state of the CPU
		Eventually allow for randomization of initial memory
	*/
	void clearState(){
		if(Config::getKeyState("CLEAR_STATE") == "true"){
			Registers.A = 0;
			Registers.F = 0;
			Registers.B = 0;
			Registers.C = 0;
			Registers.D = 0;
			Registers.E = 0;
			Registers.H = 0;
			Registers.L = 0;
			Registers.PC = 0;
		} else {
			Registers.A = 0x11;
			Registers.F = 0x80;
			Registers.B = 0x00;
			Registers.C = 0x00;
			Registers.D = 0xFF;
			Registers.E = 0x56;
			Registers.H = 0x00;
			Registers.L = 0x0D;
			Registers.PC = 0x100;
			RAM::RAM[0xFF40] = 0x91;
		}
		Registers.SP = 0xFFFE;
		Registers.IME = false;
		//shouldDisableInts = false;
		shouldEnableInts = false;
	}


	/*
		Read and write to the special registers
		For convenience
	*/
	char16_t readAF(){
		return Registers.A * 0x100 + Registers.F;
	}

	char16_t readHL(){
		return Registers.H * 0x100 + Registers.L;
	}

	char16_t readBC(){
		return Registers.B * 0x100 + Registers.C;
	}

	char16_t readDE(){
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

	inline void writeAF(char16_t data){
		unsigned char A = data / 0x100,
					  F = data - F*0x100;
		Registers.A = A;
		Registers.F = F;

	}

	inline void copyToC(int mask){
		if(readC != mask){
			Registers.F ^= 0x10;
		}
	}

	//  https://old.reddit.com/r/EmuDev/comments/9gviqn/gameboy_unit_tests/
	inline void DAA() {
		char16_t a = CPU::Registers.A;
		if(readN){
			if(readH){
				a -= 0x06;
				if(!readC){
					a &= 0xFF;
				}
			}
		}else {
			if(readH || (a & 0x0F) >= 0xA){
				a += 0x06;
			}
		}

		if(readN){
			if(readC){
				a -= 0x60;
			}
		} else {
			if(readC || a >= 0xA0){
				a += 0x60;
			}
		}

		CPU::Registers.A = (uint8_t)a;

		CPU::Registers.F |= (a > 0xFF) << 4;
		unsetH;
		if(CPU::Registers.A == 0){
			setZ;
		} else {
			unsetZ;
		}
	}

	inline void EI() {
		shouldEnableInts = true;
		//shouldDisableInts = false;
	}

	inline void DI() {
		//shouldDisableInts = true;
		//shouldEnableInts = false;
		CPU::Registers.IME = false;
	}


}  // namespace CPU

