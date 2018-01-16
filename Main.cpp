#include <iostream>
#include "File/File.hpp"
#include "Memory/RAM.hpp"
#include "Math/Math.hpp"
#include "Config/Config.hpp"
#include "CPU/CPU.hpp"

int main(int argc, char* argv[]){
	Config::setDefaults();
	Config::parseArgs(argv, argc);
	if(Config::getKeyState("ROM_LOCATION") == ""){
		return -1;
	}

	std::cout << "Debug: " << Config::getKeyState("DEBUG_MODE") << "\n";
	std::cout << "Bootrom location: " << Config::getKeyState("BOOTROM_LOCATION") << "\n";

	std::vector<unsigned char> bootRom = File::loadFromFile(Config::getKeyState("BOOTROM_LOCATION"));
	std::vector<unsigned char> rom = File::loadFromFile(Config::getKeyState("ROM_LOCATION"));
	std::cout << "File size " << rom.size() << " bytes\n";
	std::cout << "Boot rom " << bootRom.size() << " bytes\n";
	std::cout << "Trying to insert into mem...\n";
	RAM::insert(rom, 0x0150, 0x3EB0);
	RAM::insert(bootRom, 0x0, 0x100);
	RAM::dump("memdmp.txt");
	//  std::cout << "Starting CPU";
	//  CPU::start();

	return 0;
}
