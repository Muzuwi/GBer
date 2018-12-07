#pragma once
#include <vector>
#include <fstream>
#include <map>

namespace RAM{
	extern std::vector<unsigned char> RAM;
	extern std::vector<unsigned char> romFile;
	extern std::vector<unsigned char> bootRom;
	extern int mountedBankNumber;

	unsigned char read(char16_t);
	bool write(char16_t, unsigned char);
	bool write_16(char16_t, char16_t);
	bool copy(char16_t, char16_t, unsigned int);
	bool move(char16_t, char16_t, unsigned int);	
	bool insert(std::vector<unsigned char>, char16_t, unsigned int, int, bool);
	bool dump(std::string);
	void clear();
	void decodeHeader();
	void mountMemoryBanks();
	bool handleMBCread(char16_t);
	bool handleMBCwrite(char16_t, unsigned char);

	enum MBC_Type{
		ROM = 0x0,
		MBC1 = 0x01,
		MBC1_RAM = 0x02,
		MBC1_RAM_BAT = 0x03,
		MBC2 = 0x05,
		MBC2_BAT = 0x06,
		ROM_RAM = 0x08,
		ROM_RAM_BAT = 0x09,
		MMM01 = 0x0B,
		MMM01_RAM = 0x0C,
		MMM01_RAM_BAT = 0x0D,
		MBC3_TIM_BAT = 0x0F,
		MBC3_RAM_TIM_BAT = 0x10,
		MBC3 = 0x11,
		MBC3_RAM = 0x12,
		MBC3_RAM_BAT = 0x13,
		MBC5 = 0x19,
		MBC5_RAM = 0x1A,
		MBC5_RAM_BAT = 0x1B,
		MBC5_RUMBLE = 0x1C,
		MBC5_RAM_RUMBLE = 0x1D,
		MBC5_RAM_BAT_RUMBLE = 0x1E,
		MBC6_RAM_BAT = 0x20,
		MBC7_RAM_BAT_ACCEL = 0x22,
		POCKET_CAM = 0xFC,
		TAMA5 = 0xFD,
		HUC3 = 0xFE,
		HUC1_RAM_BAT = 0xFF,
	};

	enum BankingMode{
		ROM_BANK,
		RAM_BANK,
		RTC_BANK
	};
	extern MBC_Type controllerType;
	extern std::map<MBC_Type, std::string> controllerTypeLabel;
	extern BankingMode currentBankingMode;
	extern int mountedBankNumber;
	extern bool ramEnabled;
}