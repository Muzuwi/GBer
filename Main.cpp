#include <thread>
#include <iostream>
#include "File/File.hpp"
#include "Memory/RAM.hpp"
#include "Math/Math.hpp"
#include "Config/Config.hpp"
#include "CPU/CPU.hpp"
#include "Debug/Debug.hpp"
#include "CPU/Screen.hpp"

int main(int argc, char* argv[]){ 
	Config::setDefaults();
	if(!Config::parseArgs(argv, argc)){
		return -1;
	}

	Debug::emuLog("Debug: " + Config::getKeyState("DEBUG_MODE") );
	if(Config::getKeyState("CUSTOMBOOT") == "true"){
		Debug::emuLog("Using custom boot image");
		Debug::emuLog("Start rom location: " + Config::getKeyState("CUSTOMBOOT_LOCATION") );
		RAM::bootRom = File::loadFromFile(Config::getKeyState("CUSTOMBOOT_LOCATION"));
	}else{
		Debug::emuLog("Bootrom location: " + Config::getKeyState("BOOTROM_LOCATION") );
		RAM::bootRom = File::loadFromFile(Config::getKeyState("BOOTROM_LOCATION"));
	}

	Debug::emuLog("Boot rom " + std::to_string(RAM::bootRom.size()) + " bytes");
	Debug::emuLog("Loading Bootrom into RAM..");
	RAM::insert(RAM::bootRom, 0x0, RAM::bootRom.size(), 0, true);

	if(Config::getKeyState("SKIP_ROM") != "true"){
		RAM::romFile = File::loadFromFile(Config::getKeyState("ROM_LOCATION"));
		Debug::emuLog("ROM File size " + std::to_string(RAM::romFile.size()) + " bytes");
		Debug::emuLog("Loading app header into RAM..");
		RAM::insert(RAM::romFile, 0x100, 0x50, 0x100, false);
	}

	//  Init graphics
    bool ret = SDL_Init(SDL_INIT_EVERYTHING);
	if(ret != 0){
	    Debug::emuLog("SDL failed to start, emulator will run without a GUI", Debug::ERR);
	    Config::setKeyState("DEBUG_MODE", "false");
	}
    if(Config::getKeyState("DEBUG_MODE") == "true"){
		std::thread debuggerThread(&Debug::DebugWindowHandler);
		debuggerThread.detach();
	}
	std::thread gbScreenThread(&Screen::gbScreenHandler);
    gbScreenThread.detach();

	Debug::emuLog("Starting CPU");
	CPU::Registers.PC = 0x100;
	CPU::start();

	return 0;
}
