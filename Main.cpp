#include <iostream>
#include "File/File.hpp"
#include "Memory/RAM.hpp"
#include "Math/Math.hpp"

int main(int argc, char* argv[]){
	if(argc < 2){
		return -1;
	}
	std::vector<unsigned char> bootRom = File::loadFromFile("DMG_ROM.bin");
	std::vector<unsigned char> rom = File::loadFromFile(argv[1]);
	std::cout << "File size " << rom.size() << " bytes\n";
	std::cout << "Boot rom " << bootRom.size() << " bytes\n";
	std::cout << "Trying to insert into mem...";
	RAM::insert(rom, 0x0150, 0x3EB0);
	RAM::insert(bootRom, 0x0, 0x100);
	RAM::dump("memdmp.txt");

	return 0;
}