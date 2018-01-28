#include <iostream>
#include "File/File.hpp"
#include "Memory/RAM.hpp"
#include "Math/Math.hpp"
#include "Config/Config.hpp"
#include "CPU/CPU.hpp"

int main(int argc, char* argv[]){ 
	Config::setDefaults();
	if(!Config::parseArgs(argv, argc)){
		return 0;
	}
/*	if(Config::getKeyState("ROM_LOCATION") == "" && Config::getKeyState("SKIP_ROM") == "false"){
		return -1;
	}*/

	std::cout << "Debug: " << Config::getKeyState("DEBUG_MODE") << "\n";
	std::vector<unsigned char> bootRom;

	if(Config::getKeyState("CUSTOMBOOT") == "true"){
		bootRom = File::loadFromFile(Config::getKeyState("CUSTOMBOOT_LOCATION"));
		std::cout << "Using custom boot image\n";
		std::cout << "Start rom location: " << Config::getKeyState("CUSTOMBOOT_LOCATION") << "\n";
	}else{
		std::cout << "Bootrom location: " << Config::getKeyState("BOOTROM_LOCATION") << "\n";
		bootRom = File::loadFromFile(Config::getKeyState("BOOTROM_LOCATION"));
	}

	std::cout << "Boot rom " << bootRom.size() << " bytes\n";
	std::cout << "Loading Bootrom into RAM..";
	if(RAM::insert(bootRom, 0x0, bootRom.size(), true)){
		std::cout <<" yes?\n";
	}

	if(Config::getKeyState("SKIP_ROM") != "true"){
		RAM::romFile = File::loadFromFile(Config::getKeyState("ROM_LOCATION"));
		std::cout << "ROM File size " << RAM::romFile.size() << " bytes\n";
		//std::cout << "Loading app ROM into RAM..\n";
		//RAM::insert(rom, 0x0100, 0x3EB0);
	}

	std::cout << "Dumping initial RAM contents..\n";
	RAM::dump("memdmp.txt");
	std::cout << "Starting CPU\n";

	CPU::Registers.PC = 0x100;
	CPU::start();

	return 0;
}
