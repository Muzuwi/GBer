#pragma once
namespace RAM{
	extern std::vector<char> RAM(0x10000);

	char read(char16_t);
	bool write(char16_t, char);
	bool copy(char16_t, char16_t, unsigned int);
	bool move(char16_t, char16_t, unsigned int);	

}