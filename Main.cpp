#include <iostream>
#include <boost/lexical_cast.hpp>
#include "File/File.hpp"
#include "Memory/RAM.hpp"
#include "Math/Math.hpp"
#include "Config/Config.hpp"
#include "CPU/CPU.hpp"
#include "Debug/Debug.hpp"
#include <SFML/System.hpp>


int main(int argc, char* argv[]){ 
	Config::setDefaults();
	if(!Config::parseArgs(argv, argc)){
		return -1;
	}
/*	if(Config::getKeyState("ROM_LOCATION") == "" && Config::getKeyState("SKIP_ROM") == "false"){
		return -1;
	}*/

	Debug::emuLog("Debug: " + Config::getKeyState("DEBUG_MODE") );
	std::vector<unsigned char> bootRom;

	if(Config::getKeyState("CUSTOMBOOT") == "true"){
		Debug::emuLog("Using custom boot image");
		Debug::emuLog("Start rom location: " + Config::getKeyState("CUSTOMBOOT_LOCATION") );
		bootRom = File::loadFromFile(Config::getKeyState("CUSTOMBOOT_LOCATION"));
	}else{
		Debug::emuLog("Bootrom location: " + Config::getKeyState("BOOTROM_LOCATION") );
		bootRom = File::loadFromFile(Config::getKeyState("BOOTROM_LOCATION"));
	}

	Debug::emuLog("Boot rom " + boost::lexical_cast<std::string>(bootRom.size()) + " bytes");
	Debug::emuLog("Loading Bootrom into RAM..");
	RAM::insert(bootRom, 0x0, bootRom.size(), true);

	if(Config::getKeyState("SKIP_ROM") != "true"){
		RAM::romFile = File::loadFromFile(Config::getKeyState("ROM_LOCATION"));
		Debug::emuLog("ROM File size " + boost::lexical_cast<std::string>(RAM::romFile.size()) + " bytes");
		//std::cout << "Loading app ROM into RAM..\n";
		//RAM::insert(rom, 0x0100, 0x3EB0);
	}

	sf::Thread gfxone(&Debug::debugWindow);
	gfxone.launch();

	Debug::emuLog("Starting CPU");
	CPU::Registers.PC = 0x100;
	CPU::start();

	return 0;
}
