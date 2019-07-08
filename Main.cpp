#include "Headers/Emulator.hpp"

int main(int argc, char* argv[]){
    Emulator GBer;
    //  Bind modules
    GBer.init();
    //  Set configuration
    if(!GBer.getConfig()->parseArgs(argv, argc)){
        return -1;
    }

    GBer.getDebugger()->emuLog("GBer Rewrite starting");
    if (GBer.getConfig()->isDebug()){
        GBer.getDebugger()->emuLog("Debug mode is enabled");
    } else {
        GBer.getDebugger()->emuLog("Debug mode is disabled");
    }
    GBer.getDebugger()->emuLog("Filename: " + GBer.getConfig()->getFilename());
    //  Load ROM file
    GBer.getMemory()->loadROM(GBer.getConfig()->getFilename());
    GBer.getDebugger()->emuLog("ROM file size: " + std::to_string(GBer.getMemory()->getSizeROM()) + " bytes");

    //  Parse ROM header
    GBer.getMemory()->decodeHeader();

    GBer.getDebugger()->emuLog("Mounting ROM");
    GBer.getMemory()->mountBanksRAM();

    GBer.getDebugger()->emuLog("Starting emulator");
    GBer.start();

	return 0;
}
