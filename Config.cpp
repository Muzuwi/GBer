#include <string>
#include <iostream>
#include "Headers/Config.hpp"

#define GBER_CURRENT_VER "dev0.4.0"

bool Config::parseArgs(char **argv, int argc) {
    if(argc == 1) return false;
    for(int i = 1; i < argc; i++){
        if(!std::string("-ver").compare(argv[i])){
            std::cout << "GBer-" << GBER_CURRENT_VER << "\n";
            return false;
        }else if(!std::string("-help").compare(argv[i]) || !std::string("-h").compare(argv[i])){
            std::cout << "GBer version " << GBER_CURRENT_VER << "\n";
            std::cout << "Usage: GBer (-[h]elp) [argument] [option]\n\n";
            std::cout << "Options:\n";
            std::cout << " -h -help 				Prints this screen\n";
            std::cout << "    -bootrom 				Specify the bootrom file to load (must be 256 bytes)\n";
            std::cout << "    -rom 				Specify the ROM file to load\n";
            std::cout << "    -debug				Enables debugging features\n";
            std::cout << "    -ver				Prints the version of GBer\n";
            std::cout << "    -skiprom				Skips loading external ROM (Boot only)\n";
            std::cout << "    -replaceboot			Replaces the bootrom with a custom image (arbitrary length)\n";
            std::cout << "    -log				Logs the emulator state to a file\n";
            std::cout << "    -clearstate				Clears CPU registers completely on boot\n";
            std::cout << "    -bootromdebug 			Clears CPU registers & inserts only the header of rom file into RAM\n";
            std::cout << "    -nolog				Disables debug logs\n";
            std::cout << "    -nogfx				Disables graphical output (for mass testing)\n";
            return false;
        }else if(!std::string("-replaceboot").compare(argv[i]) && (argv[i+1] != NULL)){
            customBoot = true;
            //TODO: setKeyState("CUSTOMBOOT_LOCATION", argv[i+1]);
            //TODO: setKeyState("SKIP_ROM", "true");
        }else if(!std::string("-bootrom").compare(argv[i]) && (argv[i+1] != NULL) && !customBoot ){
            bootromFilename = argv[i+1];
            std::string temp = argv[i+1];
            temp.replace(temp.begin(), temp.end(),"");
            //  TODO: Store filename to use in loading save files
        }else if(!std::string("-debug").compare(argv[i])){
            debugMode = true;
        }else if(!std::string("-rom").compare(argv[i]) && (argv[i+1] != NULL)){
            romFilename = argv[i+1];
            if(!Utils::exists(argv[i+1])){
                std::cout << "ROM file does not exist!\n";
                return false;
            }
            std::string saveLoc = argv[i+1];
            if(saveLoc.find(".gb") != std::string::npos){
                saveLoc = saveLoc.replace(saveLoc.find(".gb"), 3, ".sav");
            } else {
                saveLoc += ".sav";
            }
            //std::cout << "Save name: " << saveLoc << "\n";
            saveFilename = saveLoc;
        }else if(!std::string("-skiprom").compare(argv[i]) && romFilename.empty()){
            //TODO: setKeyState("SKIP_ROM", "true");
        }else if (!std::string("-log").compare(argv[i])){
            //TODO: setKeyState("LOG", "true");
        }else if (!std::string("-clearstate").compare(argv[i])) {
            clearState = true;
        }else if (!std::string("-bootromdebug").compare(argv[i])) {
            //TODO: setKeyState("BOOTROM_DEBUG", "true");
            clearState = true;
        }else if (!std::string("-nolog").compare(argv[i])) {
            noLog = true;
        } else if (!std::string("-nogfx").compare(argv[i])){
            noGraphics = true;
        } else {
            //  Try setting the argument as the rom file
            if(Utils::exists(argv[i])){
                romFilename = argv[i];
                std::string saveLoc = argv[i];
                if(saveLoc.find(".gb") != std::string::npos){
                    saveLoc = saveLoc.replace(saveLoc.find(".gb"), 3, ".sav");
                } else {
                    saveLoc += ".sav";
                }
                //std::cout << "Save name: " << saveLoc << "\n";
                saveFilename = saveLoc;
            }
        }
    }
    return true;
}

//  TODO: ?
void Config::setDefaults() {

}

bool Config::isDebug() {
    return debugMode;
}

bool Config::noLogging() {
    return noLog;
}

bool Config::shouldClearState() {
    return clearState;
}

std::string Config::getFilename(){
    return romFilename;
}
std::string Config::getSavename(){
    return saveFilename;
}

bool Config::graphicsDisabled() {
    return noGraphics;
}

bool Config::setNewFilename(std::string filename) {
    //  Try setting the argument as the rom file
    if(Utils::exists(filename)){
        romFilename = filename;
        std::string saveLoc = filename;
        if(saveLoc.find(".gb") != std::string::npos){
            saveLoc = saveLoc.replace(saveLoc.find(".gb"), 3, ".sav");
        } else {
            saveLoc += ".sav";
        }
        //std::cout << "Save name: " << saveLoc << "\n";
        saveFilename = saveLoc;
        return true;
    } else {
        return false;
    }
}
