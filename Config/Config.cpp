#include "Config.hpp"
#include <unordered_map>
#include <iostream>
#include <string>
namespace Config{
	std::unordered_map<std::string, std::string> configTable;
	bool parseArgs(char* argv[], int argc){
		for(int i = 1; i < argc; i++){
			if(!std::string("-bootrom").compare(argv[i]) && (argv[i+1] != NULL)){
				setKeyState("BOOTROM_LOCATION", argv[i+1]);
			}else if(!std::string("-debug").compare(argv[i]) ){
				setKeyState("DEBUG_MODE", "true");
			}else if(!std::string("-rom").compare(argv[i]) && (argv[i+1] != NULL) ){
				setKeyState("ROM_LOCATION", argv[i+1]);
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
		//setKeyState("DEBUG_MODE", false);
		//setKeyState("DEBUG_MODE", false);
	}
}