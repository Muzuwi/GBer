#include "Headers/Debugger.hpp"
#include "Headers/Emulator.hpp"
#include "Headers/Config.hpp"

/*
 *  Binds the debugger to a new emulator object
 */
void Debugger::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

/*
 *  Logger functions
 */
void Debugger::emuLog(std::string message, LEVEL lvl){
    if(emulator->getConfig()->noLogging()) return;
    std::cout << (lvl == 10 ? "[@] " : \
					   lvl == 5 ? "[!] " : "[i] " ) << message << "\n";
}

void Debugger::emuLog(std::string message){
    if(emulator->getConfig()->noLogging()) return;
    std::cout << "[i] " << message << "\n";
    if(emulator->getConfig()->graphicsDisabled()) std::cout.flush();
}

/*
 *  Because SDL REALLY fucking hates creating windows from other threads
 */
void Debugger::createWindowSDL() {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    debuggerWindow = SDL_CreateWindow("GBer Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL | SDL_WINDOW_MAXIMIZED);
    debuggerWindowID = SDL_GetWindowID(debuggerWindow);
}

void Debugger::createDebugWindow(){
    createWindowSDL();
    if(debuggerWindow == nullptr){
        emulator->getDebugger()->emuLog("Debug::createDebugWindow/ SDL failed to start", ERR);
        emulator->getDebugger()->emuLog(SDL_GetError());
        return;
    }

    SDL_DisplayMode curr;
    SDL_GetCurrentDisplayMode(0, &curr);
    debuggerGLContext = SDL_GL_CreateContext(debuggerWindow);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    //  Initialize GL3W
    gl3wInit();

    //  Dear Imgui bind
    IMGUI_CHECKVERSION();
    context = ImGui::CreateContext();
    ImGui_ImplSDL2_InitForOpenGL(debuggerWindow, debuggerGLContext);
    ImGui_ImplOpenGL3_Init();

    ImGui::StyleColorsDark();

    //  Get IO object
    io = &ImGui::GetIO();
    (void)io;

    //  Enable docking
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    io->ConfigDockingWithShift = true;

    //  Init VRAM viewer framebuffers
    vramViewer.init();
    //soundWindow.init();
}

void Debugger::destroyDebugWindow(){
    //  Clean up vram Viewer framebuffers
    vramViewer.cleanup();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(debuggerGLContext);
    SDL_DestroyWindow(debuggerWindow);
}

inline ImGuiDockNodeFlags Debugger::startDockspace(){
    static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) window_flags |= ImGuiWindowFlags_NoBackground;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::Begin("DockSpace", nullptr, window_flags);
    ImGui::PopStyleVar(3);
    return dockspace_flags;
}

inline void Debugger::endDockspace(){
    ImGui::End();
}

inline void Debugger::setDefaultLayout(){

}

void Debugger::createDebugTooltip(const std::string text, unsigned int time){
    if(!text.empty()){
        timer = time;
        menuText = text;
    }
}

void Debugger::updateDebugWindowContents() {
    ImGui::BeginMainMenuBar();
    {
        if(ImGui::BeginMenu("File")){

            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("Windows")) {
            if(ImGui::MenuItem("VRAM Viewer") && !vramViewer.open) vramViewer.open = true;
            if(ImGui::MenuItem("I/O Viewer") && !ioViewer.open) ioViewer.open = true;
            if(ImGui::MenuItem("Breakpoints") && !breakpointControl.open) breakpointControl.open = true;
            ImGui::MenuItem("Interrupts");
            if(ImGui::MenuItem("APU") && !apuDebugger.open) apuDebugger.open = true;
            ImGui::MenuItem("PPU Status");

            ImGui::Separator();
            if(ImGui::MenuItem("Reset Layout")){
                resetDockLayout = true;
            }

            ImGui::EndMenu();
        }
        std::string temp = "Status: " + menuText;
        ImGui::TextColored(ImVec4(255, 255, 0, 255), temp.c_str());
        if(timer > 0){
            timer--;
        } else if(timer == 0){
            menuText = "";
        }
    }
    ImGui::EndMainMenuBar();

    //  Start dockspace
    ImGuiDockNodeFlags dockspaceFlags = startDockspace();
    ImGuiID dockspaceID = ImGui::GetID("DebuggerDock");
    if(ImGui::DockBuilderGetNode(dockspaceID) == NULL || resetDockLayout) {
        setDefaultLayout();
        resetDockLayout = false;
    }
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

    //  Window config goes here
    //  TODO: Replace magic numbers in RAM size
    memEdit.DrawWindow("GB RAM", emulator->getMemory()->getBaseMemoryPointer(), 0x10000);

    breakpointControl.updateWindow(io);
    cpuState.updateWindow(io, emulator->getCPU()->getRegisters());
    //  Hacky way to go around the circular dependency hell I made
    if(flowWindow.updateWindow(io, emulator->getCPU(), emulator->getDisplay())){
        emulator->triggerReload();
    }
    if(flowWindow.shouldPauseDebugger()){
        pauseDebugger();
        flowWindow.clearShouldPauseDebugger();
    }
    //  If continue was clicked, we pause the debugger until it is brought up again
    ioViewer.updateWindow(io, emulator->getCPU(), emulator->getMemory());
    ppuWindow.updateWindow(io, emulator->getPPU(),emulator->getMemory(), emulator->getDisplay());
    //soundWindow.updateWindow(io);
    stackWindow.updateWindow(io, emulator->getCPU(), emulator->getMemory());
    vramViewer.updateWindow(io, emulator->getPPU(), emulator->getMemory());
    interruptsWindow.updateWindow(io, emulator->getCPU(), emulator->getMemory());
    performanceDebugger.updateWindow(io);
    apuDebugger.updateWindow(io, emulator->getMemory());

    //  End dockspace
    endDockspace();
}

void Debugger::updateDebugWindow(){
    ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    //  Start dear imgui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(debuggerWindow);
    ImGui::NewFrame();

    updateDebugWindowContents();

    ImGui::EndFrame();
    //  Render frame
    ImGui::Render();
    glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(debuggerWindow);
}

inline void Debugger::pauseDebugger() {
    isPaused = true;
    SDL_SetWindowTitle(debuggerWindow, "GBer Debugger (PAUSED)");
}

inline void Debugger::unpauseDebugger() {
    isPaused = false;
    SDL_SetWindowTitle(debuggerWindow, "GBer Debugger");
    SDL_RaiseWindow(debuggerWindow);
}


void Debugger::handleEvent(SDL_Event* event) {
    //  TODO: Dangerous thread operation?
    ImGui::SetCurrentContext(context);
    ImGui_ImplSDL2_ProcessEvent(event);
    switch(event->type){
        case SDL_WINDOWEVENT:
            if(event->window.event == SDL_WINDOWEVENT_CLOSE){
                emulator->halt();
            } else if(event->window.event == SDL_WINDOWEVENT_FOCUS_LOST){
                pauseDebugger();
                emuLog("Pausing the debugger");
                //  Disable step mode
            } else if(event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED){
                unpauseDebugger();
                emuLog("Resuming the debugger");
                //  Set continueExec to false, enable step mode
                emulator->getCPU()->setContinueExec(false);
                emulator->getCPU()->setStep(false);
            }
            break;
        case SDL_KEYDOWN:
            if(event->key.keysym.sym == SDLK_F3){
                emulator->getCPU()->setStep(true);
            }
        default: {
            //std::cout << "Unknown event " << event.type << "\n";
            break;
        }
    }
}

unsigned int Debugger::getWindowID() {
    return debuggerWindowID;
}

bool Debugger::isDebuggerPaused() {
    return isPaused;
}

void Debugger::handleAddressBreakpoint(uint16_t address) {
    if(breakpointControl.checkAddressBreakpoint(address)){
        emulator->getCPU()->setStep(false);
        emulator->getCPU()->setContinueExec(false);
        createDebugTooltip("Reached breakpoint at 0x" + Utils::decHex(address), 180);
        if(isPaused){
            unpauseDebugger();
        }
    }
}

void Debugger::handleInstructionBreakpoint(uint8_t op) {
    if(breakpointControl.checkInstructionBreakpoint(op)){
        emulator->getCPU()->setStep(false);
        emulator->getCPU()->setContinueExec(false);
        createDebugTooltip("Instruction breakpoint $" + Utils::decHex(op), 180);
        if(isPaused){
            unpauseDebugger();
        }
    }
}

void Debugger::handleMemoryBreakpoint(uint16_t address) {
    if(breakpointControl.checkMemoryBreakpoint(address)) {
        emulator->getCPU()->setStep(false);
        emulator->getCPU()->setContinueExec(false);
        createDebugTooltip("Memory read at $" + Utils::decHex(address), 180);
        if(isPaused){
            unpauseDebugger();
        }
    }
}

void Debugger::handleMemoryBreakpoint(uint16_t address, uint8_t byte) {
    if(breakpointControl.checkMemoryBreakpoint(address)) {
        emulator->getCPU()->setStep(false);
        emulator->getCPU()->setContinueExec(false);
        createDebugTooltip("Memory write at $" + Utils::decHex(address) + " [" + Utils::decHex(byte, 2) + "]", 180);
        if(isPaused){
            unpauseDebugger();
        }
    }
}
