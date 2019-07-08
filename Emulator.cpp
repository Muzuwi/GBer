#include <thread>
#include <SDL2/SDL.h>
#include <future>
#include "Headers/RAM.hpp"
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
}

void Emulator::start(){
    if(!config.graphicsDisabled()){
        display.initializeGraphicLibs();
        display.createGameWindow();
    }

    //  Launch debug thread
    if(config.isDebug()){
        debug.createDebugWindow();
    }

    //  Reset CPU state
    cpu.start();

    //  My patent-pending anti-undefined-behavior-device (tm)
    display.setFrameTime();

    //  Main emu loop
    while(!emuHalt){
        if(shouldReload) reload();

        //  Cycle only when not in pause mode

        int64_t taken = 0;
        if(cpu.doStep()) taken = cpu.cycle();

        //  Update the DIV registers
        cpu.updateTimers(taken);

        //  Update PPU-related variables
        ppu.updateVariables();

        bool drawFrame = ppu.updateModePPU(taken);
        //  If LCD is enabled
        if((ppu.getLCDC()->lcdEnable || (!cpu.doStep())) && !config.graphicsDisabled()){
            //  Update PPU status
            //  Draw new frame when ready
            if(drawFrame || (!cpu.doStep())){
                //  Update main display and the debugger window
                display.updateWindow();
            }
        }

        //  Update events on frame update, when in step mode or when in STOP
        if((cpu.isStopped() || drawFrame || (!cpu.doStep())) && !config.graphicsDisabled()){
            handleEventsSDL();
        }

        //  If a file dropped event has occured
        if(romChangeRequested){
            //  Unmount RAM and save flash to disk
            memory.unmountRAM();

            //  Check if the file is valid
            if(!config.setNewFilename(newRomFile)){
                debug.emuLog("Failed to load ROM! " + newRomFile + ".");
                romChangeRequested = false;
                newRomFile = "";
                //  TODO: Graceful shutdown
                std::terminate();
            }

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

            continue;
        }

        //  Draw the debug window if unpaused
        if(config.isDebug() && !debug.isDebuggerPaused()){
            debug.updateDebugWindow();
        }

        //  Limit framerate
        if(drawFrame){
            if(display.getFrameDuration() < 1000.0/60.0){
                std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000.0/60.0)-display.getFrameDuration()));
            }
            display.setFrameTime();
        }

    }

    display.destroyGameWindow();
    if(config.isDebug()){
        debug.destroyDebugWindow();
    }
    memory.unmountRAM();

    debug.emuLog("Bye bye!");
    SDL_Quit();
}

void Emulator::reload(){
    cpu.reload();
    memory.reload();
    ppu.reload();
    display.reload();
    debug.createDebugTooltip("Reload completed", 120);
    shouldReload = false;
}

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

void Emulator::halt(){
    emuHalt = true;
}

bool Emulator::isHalted(){
    return emuHalt;
}

void Emulator::triggerReload() {
    shouldReload = true;
}

/*
 *  Manages SDL events across both threads (Main/Debugger)
 */
void Emulator::handleEventsSDL() {
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        //  Dispatch events to window event handlers
        if(event.window.windowID == display.getWindowID() || (event.type == SDL_DROPFILE && event.drop.windowID == display.getWindowID())) display.handleEvent(event);
        else if(config.isDebug() && event.window.windowID == debug.getWindowID()) {
            debug.handleEvent(event);
        } else {
            if(event.type == SDL_QUIT) {
                debug.emuLog("SIGQUIT received!");
                halt();
            }
            continue;
        }
    }
}

void Emulator::requestChangeROM(std::string newROM) {
    newRomFile = newROM;
    romChangeRequested = true;
}
