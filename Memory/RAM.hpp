#pragma once
#include <vector>
#include <fstream>

namespace RAM{
	extern std::vector<unsigned char> RAM;
	extern std::vector<unsigned char> romFile;

	char read(char16_t);
	bool write(char16_t, unsigned char);
	bool write_16(char16_t, char16_t);
	bool copy(char16_t, char16_t, unsigned int);
	bool move(char16_t, char16_t, unsigned int);	
	bool insert(std::vector<unsigned char>, char16_t, unsigned int);
	bool dump(std::string);
}