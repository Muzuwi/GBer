#define CURRENT_VER "dev0.0.3"

#include "Config.hpp"
#include <unordered_map>
#include <iostream>
#include <string>

namespace Config{
	std::unordered_map<std::string, std::string> configTable;
	bool parseArgs(char* argv[], int argc){
		for(int i = 1; i < argc; i++){
			if(!std::string("-ver").compare(argv[i])){
				std::cout << "GBer-" << CURRENT_VER << "\n";
			}else if(!std::string("-help").compare(argv[i]) || !std::string("-h").compare(argv[i])){
				std::cout << "GBer version " << CURRENT_VER << "\n";
				std::cout << "Usage: GBer (-[h]elp) [argument] [option]\n\n";
				std::cout << "Options:\n";
				std::cout << " -h -help 				Prints this screen\n";
				std::cout << "    -bootrom 				Specify the bootrom file to load (must be 256 bytes)\n";
				std::cout << "    -rom 				Specify the ROM file to load\n";
				std::cout << "    -debug				Enables debugging features\n";
				std::cout << "    -ver				Prints the version of GBer\n";
				std::cout << "    -skiprom				Skips loading external ROM (Boot only)\n";
				std::cout << "    -replaceboot			Replaces the bootrom with a custom image (arbitrary length)\n";
				return false;
			}else if(!std::string("-replaceboot").compare(argv[i]) && (argv[i+1] != NULL)){
				setKeyState("CUSTOMBOOT", "true");
				setKeyState("CUSTOMBOOT_LOCATION", argv[i+1]);
				setKeyState("SKIP_ROM", "true");
			}else if(!std::string("-bootrom").compare(argv[i]) && (argv[i+1] != NULL) && getKeyState("CUSTOMBOOT") != "true" ){
				setKeyState("BOOTROM_LOCATION", argv[i+1]);
			}else if(!std::string("-debug").compare(argv[i])){
				setKeyState("DEBUG_MODE", "true");
			}else if(!std::string("-rom").compare(argv[i]) && (argv[i+1] != NULL)){
				setKeyState("ROM_LOCATION", argv[i+1]);
			}else if(!std::string("-skiprom").compare(argv[i]) && getKeyState("ROM_LOCATION") == ""){
				setKeyState("SKIP_ROM", "true");
			}
		}
		return true;
	}

	std::string getKeyState(std::string key){
		return configTable[key];
	}

	void setKeyState(std::string key, std::string state){
		configTable[key] = state;
	}

	void setDefaults(){
		setKeyState("DEBUG_MODE", "false");
		setKeyState("BOOTROM_LOCATION", "DMG_ROM.bin");
		setKeyState("SKIP_ROM", "false");
		//setKeyState("ROM_LOCATION", "pkb.gb");
	}
}  // namespace Config
