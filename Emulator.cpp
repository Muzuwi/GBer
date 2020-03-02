#include <thread>
#include <SDL2/SDL.h>
#include <Headers/Display.hpp>
#include "Headers/Emulator.hpp"

/*
 *  Binds the child classes to this instance of Emulator
 */
void Emulator::init(){
    memory.bind(this);
    ppu.bind(this);
    debug.bind(this);
    display.bind(this);
    cpu.bind(this);
    apu.bind(this);
}

/*
 *  Runs the emulator and enters the main emu loop
 */
void Emulator::start(){
    //  Initialize graphic libraries and create main emulator window
    if(!config.graphicsDisabled()){
        display.initialize();
    }

    apu.init();

    //  Reset CPU state
    cpu.start();

    //  Main emu loop
    while(!emuHalt){
        if(shouldReload) reload();

        //  Cycle only when not in pause mode
        int64_t taken = 0;
        if(cpu.doStep()) taken = cpu.cycle();

        //  Update the DIV registers
        cpu.updateTimers(taken);

        //  Advance any memory transfers
        memory.updateTransfers(taken);

        //  Update PPU-related variables
        ppu.updateVariables();

        //  Update APU
        apu.update();

        //  Update PPU status
        bool drawFrame = ppu.updateModePPU(taken);
        //  If LCD is enabled
        if(ppu.getLCDC()->lcdEnable && !config.graphicsDisabled()){
            //  Draw new frame when ready
            if(drawFrame){
                SDL_Event event;
                while(SDL_PollEvent(&event)){
                    display.handleEvent(&event);
                }
                display.updateWindow();
            }
        }

        //  If in step mode, update the frame immediately
        if(!cpu.doStep()){
            SDL_Event event;
            while(SDL_PollEvent(&event)){
                display.handleEvent(&event);
            }
            display.updateWindow();
        }

        //  If a file dropped event has occured, try changing the ROM
        if(romChangeRequested){
            //  If the change was successful, start from the beginning
            if(handleChangeROM()) continue;
        }

        //  Limit framerate
        if(drawFrame && ppu.getLCDC()->lcdEnable && !config.graphicsDisabled() && !config.isDebug()){
            auto time = display.updateFrameTime();
            auto target = 1000.0/60.0;
            if(time.count() < target){
                auto sleepFor = target - time.count();
                auto ms = std::chrono::duration<double, std::milli>(sleepFor);
                std::this_thread::sleep_for(ms);
            }
        }
    }
    /*
     *  End of main emu loop
     */

    display.terminate();
    memory.unmountRAM();

    debug.emuLog("Bye bye!");
}

/*
 *  Triggers a reload of all the emulator submodules, i.e. reboots the rom
 */
void Emulator::reload(){
    cpu.reload();
    memory.reload();
    ppu.reload();
    display.reload();
    display.createDebugTooltip("Reload completed", 120);
    shouldReload = false;
}

/*
 *  Getters for the emulator submodules
 */

Config* Emulator::getConfig(){
    return &config;
}

Debugger* Emulator::getDebugger(){
    return &debug;
}

PPU* Emulator::getPPU(){
    return &ppu;
}

RAM* Emulator::getMemory(){
    return &memory;
}

Display* Emulator::getDisplay(){
    return &display;
}

LR35902* Emulator::getCPU() {
    return &cpu;
}

/*
 *  Trigger a halt state
 */
void Emulator::halt(){
    emuHalt = true;
}

/*
 *  Trigger an emulator reload
 */
void Emulator::triggerReload() {
    shouldReload = true;
}

/*
 *  Enable/disable the debugger
 */
void Emulator::triggerToggleDebugger() {
    config.toggleDebug();
    display.eventDebugStatusSwitched();
}


/*
 *  Called when a new rom file is dropped onto the emulator window
 *  Sets the new rom file name and flag
 */
void Emulator::requestChangeROM(std::string newROM) {
    newRomFile = newROM;
    romChangeRequested = true;
}

/*
 *  Called when a ROM change is requested, by a drag-and-drop event or otherwise.
 *  Returns true when the ROM was loaded successfully, otherwise false
 */
bool Emulator::handleChangeROM() {
    //  Check if the file is valid
    if(!config.setNewFilename(newRomFile)){
        //  File invalid, cancel load
        debug.emuLog("Failed to load ROM! " + newRomFile + ".");
        romChangeRequested = false;
        newRomFile = "";
        return false;
    } else {
        //  File valid, start loading new ROM
        //  Unmount current RAM and save flash to disk
        memory.unmountRAM();

        debug.emuLog("#############################################");
        //  Reload state
        debug.emuLog("Loading " + newRomFile);
        //  Load ROM file
        memory.loadROM(config.getFilename());
        debug.emuLog("Filename: " + config.getFilename());
        debug.emuLog("ROM file size: " + std::to_string(memory.getSizeROM()) + " bytes");
        //  Parse ROM header
        debug.emuLog("Decoding new ROM header..");
        memory.decodeHeader();
        debug.emuLog("Reloading the emulator");
        reload();

        debug.emuLog("Mounting ROM");
        memory.mountBanksRAM();

        debug.emuLog("ROM change successful!");

        romChangeRequested = false;
        newRomFile = "";

        return true;
    }
}

/*
 *  Triggers a breakpoint with a specified message
 */
void Emulator::triggerBreak(std::string message) {
    cpu.setStep(false);
    cpu.setContinueExec(false);
    display.createDebugTooltip(message, 180);
}

APU *Emulator::getAPU() {
    return &apu;
}

void Emulator::die(std::string reason) {
    debug.emuLog("GBer has crashed!", LOGLEVEL::ERR);
    debug.emuLog("Reason: " + reason, LOGLEVEL::ERR);
    emuHalt = true;
}
