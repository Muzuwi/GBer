#include "Headers/Display.hpp"
#include "Headers/Emulator.hpp"

//  Bind a new emulator object to this class
void Display::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

/*
 *  Init SDL
 */
bool Display::initializeGraphicLibs() {
    int ret = SDL_Init(SDL_INIT_EVERYTHING);
    if (ret != 0) {
        emulator->getDebugger()->emuLog("SDL failed to start, emulator will run without a GUI", emulator->getDebugger()->ERR);
        //  TODO: Set debug mode
        graphicsInitializationFailure = true;
        return false;
    }
    return true;
}

/*
 *  Draw updated window contents
 */
void Display::updateWindow(){
    /*for(size_t i = 0; i < GAMEBOY_SCREEN_WIDTH*GAMEBOY_SCREEN_HEIGHT; i++){
        framebuffer[i] = (uint32_t)0xF0FF00FF;
    }*/

    SDL_Texture* buffer = SDL_CreateTexture(gberRenderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT);
    SDL_Rect output;
    output.x = 0;
    output.y = 0;
    output.w = GAMEBOY_SCREEN_WIDTH;
    output.h = GAMEBOY_SCREEN_HEIGHT;

    SDL_UpdateTexture(buffer, NULL, framebuffer, GAMEBOY_SCREEN_WIDTH * sizeof(uint32_t) );

    SDL_RenderCopy(gberRenderer, buffer, NULL, &output);

    SDL_DestroyTexture(buffer);

    SDL_RenderPresent(gberRenderer);
}

/*
 *  Copy pixel data of a line from PPU framebuffer to display framebuffer
 */
void Display::appendBufferedLine(uint32_t* lineData, size_t size, size_t line){
    assert(line >= 0 && line <= GAMEBOY_SCREEN_HEIGHT);
    memcpy(&framebuffer[line*GAMEBOY_SCREEN_WIDTH], lineData, size*sizeof(uint32_t));
}

/*
 *  Initialize the SDL window and renderer
 */
void Display::createGameWindow(){ //  0  85   170   255
    bool ret;
    gameboyColors[0].r = gameboyColors[0].g = gameboyColors[0].b = 255;
    gameboyColors[1].r = gameboyColors[1].g = gameboyColors[1].b = 170;
    gameboyColors[2].r = gameboyColors[2].g = gameboyColors[2].b = 85;
    gameboyColors[3].r = gameboyColors[3].g = gameboyColors[3].b = 0;
    gberWindow = SDL_CreateWindow("GBer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAMEBOY_SCREEN_WIDTH*scale, GAMEBOY_SCREEN_HEIGHT*scale, SDL_WINDOW_OPENGL);
    gberRenderer = SDL_CreateRenderer(gberWindow, -1, SDL_RENDERER_ACCELERATED);
    gberWindowID = SDL_GetWindowID(gberWindow);
    if(gberRenderer == nullptr || gberWindow == nullptr){
        emulator->getDebugger()->emuLog(SDL_GetError(), emulator->getDebugger()->ERR);
        ret = false;
    } else {
        SDL_RenderSetScale(gberRenderer, scale, scale);
        ret = true;
    }
    graphicsInitializationFailure = ret;
}

/*
 *  Destroy SDL window and renderer, free buffers
 */
void Display::destroyGameWindow(){
    SDL_DestroyRenderer(gberRenderer);
    SDL_DestroyWindow(gberWindow);
}

/*
 *  Clears the SDL window
 */
void Display::clearWindow(){
    SDL_SetRenderDrawColor(gberRenderer, gameboyColors[0].r, gameboyColors[0].g, gameboyColors[0].b, gameboyColors[0].a);
    SDL_RenderClear(gberRenderer);
    SDL_RenderPresent(gberRenderer);
}

/*
 *  Reloads the display module
 */
void Display::reload(){
    clearWindow();
}

/*
 *  Returns color n
 */
SDL_Color Display::getColor(size_t n){
    assert(n < 4);
    return gameboyColors[n];
}

/*
 *  Returns the joypad state
 */
Keypad* Display::getJoypad(){
    return &joypadState;
}

int64_t Display::getFrameDuration() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(frameDuration).count();
}

void Display::setFrameTime() {
    frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastFrameTimepoint);
    lastFrameTimepoint = std::chrono::system_clock::now();
}

void Display::handleEvent(SDL_Event* event) {
    //  Better input behavior
    static bool leftHold = false,
                upHold = false,
                downHold = false,
                rightHold = false;

    switch(event->type) {
        case SDL_WINDOWEVENT:
            if(event->window.event == SDL_WINDOWEVENT_CLOSE){
                emulator->halt();
            } else if(event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
                // Disable pause mode
                emulator->getCPU()->setContinueExec(true);
                emulator->getCPU()->setStep(true);
            }
            break;
        case SDL_KEYUP:
            switch(event->key.keysym.sym){
                case SDLK_z: {
                    joypadState.aP = false; break;
                }
                case SDLK_x: {
                    joypadState.bP = false; break;
                }
                case SDLK_a: {
                    joypadState.selP = false; break;
                }
                case SDLK_s: {
                    joypadState.startP = false; break;
                }
                case SDLK_UP: {
                    joypadState.uP = false;
                    upHold = false;
                    //  Restore previous input
                    if(downHold){
                        joypadState.dP = true;
                    }
                    break;
                }
                case SDLK_DOWN: {
                    joypadState.dP = false;
                    downHold = false;
                    //  Restore previous input
                    if(upHold){
                        joypadState.uP = true;
                    }
                    break;
                }
                case SDLK_LEFT: {
                    joypadState.lP = false;
                    leftHold = false;
                    //  Restore previous input
                    if(rightHold){
                        joypadState.rP = true;
                    }
                    break;
                }
                case SDLK_RIGHT: {
                    joypadState.rP = false;
                    rightHold = false;
                    //  Restore previous input
                    if(leftHold){
                        joypadState.lP= true;
                    }
                    break;
                }
                default: break;
            }
            break;
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym){
                case SDLK_z: {
                    joypadState.aP = true;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_x: {
                    joypadState.bP = true;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_a: {
                    joypadState.selP = true;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_s: {
                    joypadState.startP = true;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_UP: {
                    upHold = true;
                    joypadState.uP = true;
                    if(joypadState.dP) joypadState.dP = false;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_DOWN: {
                    downHold = true;
                    joypadState.dP = true;
                    if(joypadState.uP) joypadState.uP = false;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_LEFT: {
                    leftHold = true;
                    joypadState.lP = true;
                    if(joypadState.rP) joypadState.rP = false;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                case SDLK_RIGHT: {
                    rightHold = true;
                    joypadState.rP = true;
                    if(joypadState.lP) joypadState.lP = false;
                    emulator->getCPU()->wakeFromStop();
                    break;
                }
                default: break;
            }
            break;
        case SDL_DROPFILE:
            char* dropped;
            dropped = event->drop.file;
            //std::cout << dropped << " file\n";
            emulator->requestChangeROM(dropped);
            SDL_free(dropped);
            break;
        default: {
            break;
        }
    }
}

unsigned int Display::getWindowID() {
    return gberWindowID;
}

SDL_Window *Display::getWindow() {
    return gberWindow;
}
