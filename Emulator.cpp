#include <thread>
#include <SDL2/SDL.h>
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

/*
 *  Runs the emulator and enters the main emu loop
 */
void Emulator::start(){
    //  Initialize graphic libraries and create main emulator window
    if(!config.graphicsDisabled()){
        display.initializeGraphicLibs();
        display.createGameWindow();
    }

    //  Create debug window if in debug mode
    if(config.isDebug()){
        debug.createDebugWindow();
    }

    //  Reset CPU state
    cpu.start();

    //  My patent-pending anti-undefined-behavior-device (tm)
    display.setFrameTime();

    //  Setup callback for events
    SDL_AddEventWatch(Emulator::SDLEventAddedCallback, this);

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

        //  Update PPU status
        bool drawFrame = ppu.updateModePPU(taken);
        //  If LCD is enabled
        if(ppu.getLCDC()->lcdEnable && !config.graphicsDisabled()){
            //  Draw new frame when ready
            if(drawFrame){
                display.updateWindow();
            }
        }

        //  Fill the event queue and let the callback handle new events
        SDL_PumpEvents();

        //  If a file dropped event has occured, try changing the ROM
        if(romChangeRequested){
            //  Check if the file is valid
            if(!config.setNewFilename(newRomFile)){
                //  File invalid, cancel load
                debug.emuLog("Failed to load ROM! " + newRomFile + ".");
                romChangeRequested = false;
                newRomFile = "";
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

                continue;
            }
        }

        //  Draw the debug window if unpaused
        //if(config.isDebug() && !debug.isDebuggerPaused()){
            //debug.updateDebugWindow();
        //}
        if(config.isDebug() && (drawFrame || !cpu.doStep())){
            debug.updateDebugWindow();
        }


        //  Limit framerate
        //  TODO: FIX
        //if(drawFrame){
        //    if(display.getFrameDuration() < 1000.0/60.0){
        //        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1000.0/60.0)-display.getFrameDuration()));
        //    }
        //    display.setFrameTime();
        //}

    }

    display.destroyGameWindow();
    if(config.isDebug()){
        debug.destroyDebugWindow();
    }
    SDL_Quit();
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
    debug.createDebugTooltip("Reload completed", 120);
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

bool Emulator::isHalted(){
    return emuHalt;
}

/*
 *  Trigger an emulator reload
 */
void Emulator::triggerReload() {
    shouldReload = true;
}

/*
 *  Manages SDL events across both threads (Main/Debugger)
 */
void Emulator::handleEventsSDL() {
    //SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    //SDL_Event event;
    //while(SDL_PollEvent(&event)){
    //    //  Dispatch events to window event handlers
    //    if(event.window.windowID == display.getWindowID() || (event.type == SDL_DROPFILE && event.drop.windowID == display.getWindowID())) display.handleEvent(event);
    //    else if(config.isDebug() && event.window.windowID == debug.getWindowID()) {
    //        debug.handleEvent(event);
    //    } else {
    //        if(event.type == SDL_QUIT) {
    //            debug.emuLog("SIGQUIT received!");
    //            halt();
    //        }
    //        continue;
    //    }
    //}
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
 *  SDL event callback, processes events for the emulator windows
 *  A pointer to the emulator object is passed in 'usrData' when setting the callback
 */
int SDLCALL Emulator::SDLEventAddedCallback(void *usrData, SDL_Event *event) {
    if(usrData == nullptr) return -1;
    if(event == nullptr) return -1;

    auto display = static_cast<Emulator*>(usrData)->getDisplay();
    auto debug = static_cast<Emulator*>(usrData)->getDebugger();
    auto config = static_cast<Emulator*>(usrData)->getConfig();

    if(event->window.windowID == display->getWindowID() || (event->type == SDL_DROPFILE && event->drop.windowID == display->getWindowID())) display->handleEvent(event);
    else if(config->isDebug() && event->window.windowID == debug->getWindowID()) debug->handleEvent(event);
    else {
        if(event->type == SDL_QUIT) {
            debug->emuLog("SIGQUIT received!");
            static_cast<Emulator*>(usrData)->halt();
        }
    }
    return 0;
}
