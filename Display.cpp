#include <Headers/Display.hpp>
#include "Headers/Display.hpp"
#include "Headers/Emulator.hpp"


/*
 *  Bind a new emulator object to this class
 */
void Display::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

///
///  Initialization/termination functions
///

/*
 *  Creates the main window, launches imgui and other funky graphic stuff
 */
void Display::initialize() {
    //  Set the color pallete
    gameboyColors[0].r = gameboyColors[0].g = gameboyColors[0].b = 255;
    gameboyColors[1].r = gameboyColors[1].g = gameboyColors[1].b = 170;
    gameboyColors[2].r = gameboyColors[2].g = gameboyColors[2].b = 85;
    gameboyColors[3].r = gameboyColors[3].g = gameboyColors[3].b = 0;

    this->initializeSDL();
    this->createGameWindow();
    this->initializeImGui();
    this->DebuggerWindows.vramViewer.init();
}

/*
 *  Do i really have to explain?
 */
void Display::terminate() {
    this->terminateImGuiContext();
    this->destroyGameWindow();
    this->terminateSDL();
}

/*
 *  Init SDL
 */
bool Display::initializeSDL() {
    int ret = SDL_Init(SDL_INIT_EVERYTHING);
    if (ret != 0) {
        emulator->getDebugger()->emuLog("SDL failed to start", LOGLEVEL::ERR);
        return false;
    }
    return true;
}

/*
 *  Create an SDL window and gl context, initializes Gl3w
 */
void Display::createGameWindow(){
    bool ret;

    //  Set GL hints
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    //  Create window
    //  If in debug mode, maximize the window
    if(!emulator->getConfig()->isDebug()){
        gberWindow = SDL_CreateWindow("GBer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, GAMEBOY_SCREEN_WIDTH*scale, GAMEBOY_SCREEN_HEIGHT*scale, SDL_WINDOW_OPENGL);
    } else {
        gberWindow = SDL_CreateWindow("GBer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SDL_WINDOW_MAXIMIZED, SDL_WINDOW_MAXIMIZED, SDL_WINDOW_OPENGL|SDL_WINDOW_MAXIMIZED);
    }
    gberWindowID = SDL_GetWindowID(gberWindow);
    gberGLContext = SDL_GL_CreateContext(gberWindow);

    //  Initialize GL3W
    int error = gl3wInit();
    if (error != 0){
        emulator->getDebugger()->emuLog("Gl3w failed to start, error code " + std::to_string(error), LOGLEVEL::ERR);
    }

    //  Error checking
    if(gberWindow == nullptr){
        emulator->getDebugger()->emuLog(SDL_GetError(), LOGLEVEL::ERR);
        ret = false;
    } else {
        ret = true;
    }

    //  Create gl fbo's
    SDL_GL_MakeCurrent(gberWindow, gberGLContext);
    this->setGLState();
    //  We don't need no vsync, disable it
    SDL_GL_SetSwapInterval(0);

    graphicsInitializationFailure = ret;
}

/*
 *  Runs any necessary gl calls
 */
void Display::setGLState() {
    glGenFramebuffers(1, &this->gameboyFramebuffer);
    //  GL textures
    glGenTextures(1, &this->gameboyTexture);
    glBindTexture(GL_TEXTURE_2D, this->gameboyTexture);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);
}



/*
 *  Initializes ImGui, creates the context and sets up default configuration
 */
void Display::initializeImGui() {
    //  Init ImGui overlay
    IMGUI_CHECKVERSION();
    overlayContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(overlayContext);
    ImGui_ImplSDL2_InitForOpenGL(gberWindow, gberGLContext);
    ImGui_ImplOpenGL3_Init();
    ImGui::StyleColorsDark();

    //  When debug mode is enabled, turn on docking
    if(emulator->getConfig()->isDebug()){
        auto& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigDockingWithShift = true;
    }

    //  Set up imgui defaults
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 2.0f;
    style.TabRounding = 1.0f;
    style.ScrollbarRounding = 2.0f;
    style.WindowBorderSize = 0.0f;
    style.FrameRounding = 3.0f;

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_TextDisabled]           = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
    colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.12f, 0.12f, 0.94f);
    colors[ImGuiCol_ChildBg]                = ImVec4(0.07f, 0.07f, 0.07f, 0.00f);
    colors[ImGuiCol_FrameBg]                = ImVec4(0.43f, 0.43f, 0.43f, 0.54f);
    colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.55f, 0.55f, 0.55f, 0.54f);
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.65f, 0.65f, 0.65f, 0.54f);
    colors[ImGuiCol_TitleBg]                = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
    colors[ImGuiCol_TitleBgActive]          = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
    colors[ImGuiCol_CheckMark]              = ImVec4(0.00f, 0.86f, 0.36f, 1.00f);
    colors[ImGuiCol_SliderGrab]             = ImVec4(0.04f, 0.95f, 1.00f, 0.40f);
    colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.01f, 0.85f, 0.90f, 0.40f);
    colors[ImGuiCol_Button]                 = ImVec4(0.00f, 0.72f, 0.73f, 0.47f);
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.00f, 0.80f, 0.81f, 0.47f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 0.91f, 0.92f, 0.47f);
    colors[ImGuiCol_Header]                 = ImVec4(0.57f, 0.57f, 0.57f, 0.31f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.80f, 0.80f, 0.80f, 0.31f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.73f, 0.73f, 0.73f, 0.31f);
    colors[ImGuiCol_ResizeGrip]             = ImVec4(0.73f, 0.73f, 0.73f, 0.32f);
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.73f, 0.73f, 0.73f, 0.50f);
    colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.73f, 0.73f, 0.73f, 0.78f);
    colors[ImGuiCol_Tab]                    = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
    colors[ImGuiCol_TabHovered]             = ImVec4(0.00f, 0.77f, 0.78f, 0.47f);
    colors[ImGuiCol_TabActive]              = ImVec4(0.00f, 0.98f, 1.00f, 0.47f);
    colors[ImGuiCol_PlotLines]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.81f, 0.47f, 0.35f);
    colors[ImGuiCol_NavHighlight]           = ImVec4(0.15f, 0.72f, 0.54f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.66f, 0.66f, 0.72f, 0.50f);
    colors[ImGuiCol_SeparatorActive]        = ImVec4(0.45f, 0.45f, 0.51f, 0.50f);
    colors[ImGuiCol_TabUnfocused]           = ImVec4(0.00f, 0.58f, 0.59f, 0.47f);
    colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.03f, 0.70f, 0.71f, 0.47f);



}

/*
 *  Destroy SDL window and gl context, free buffers
 */
void Display::destroyGameWindow(){
    SDL_GL_DeleteContext(gberGLContext);
    SDL_DestroyWindow(gberWindow);
}

/*
 *  Terminates imgui subsystems
 */
void Display::terminateImGuiContext() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
}

/*
 *  Terminates SDL
 */
void Display::terminateSDL() {
    SDL_Quit();
}


///
///  Runtime functions
///

/*
 *  Update the main menu bar
 */
void Display::drawMenuBar() {
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::BeginMainMenuBar();
    ImGui::PopStyleVar(1);

    if(ImGui::BeginMenu("File")){
        if(ImGui::MenuItem("Reload ROM")) WindowFlags.reloadROM = true;
        ImGui::EndMenu();
    }

    if(ImGui::BeginMenu("Configuration")){
        if(ImGui::MenuItem("Graphics")) WindowFlags.showGraphicsConfig = true;
        if(ImGui::MenuItem("Input")) WindowFlags.showInputConfig = true;
        ImGui::EndMenu();
    }

    //  Draw debugger tabs
    if(emulator->getConfig()->isDebug()){
        if(ImGui::BeginMenu("Widgets")){
            if(ImGui::MenuItem("VRAM Viewer", nullptr, &DebuggerWindows.vramViewer.open)) DebuggerWindows.vramViewer.open = true;
            if(ImGui::MenuItem("I/O Viewer", nullptr, &DebuggerWindows.ioViewer.open)) DebuggerWindows.ioViewer.open = true;
            //if(ImGui::MenuItem("Breakpoints", nullptr, &DebuggerWindows.bre.open)) DebuggerWindows.vramViewer.open = true;
            if(ImGui::MenuItem("APU", nullptr, &DebuggerWindows.apuViewer.open)) DebuggerWindows.apuViewer.open = true;
            if(ImGui::MenuItem("PPU Status", nullptr, &DebuggerWindows.ppuViewer.open)) DebuggerWindows.ppuViewer.open = true;

            ImGui::Separator();
            if(ImGui::MenuItem("Reset widget layout")) WindowFlags.resetLayout = true;

            ImGui::EndMenu();
        }
    }

    if(ImGui::BeginMenu("About")){
        if(ImGui::MenuItem("Show ImGui demo window", nullptr, &WindowFlags.showDemoWindow)){
            WindowFlags.showDemoWindow = true;
        }
        ImGui::EndMenu();
    }

    if(ImGui::Button(emulator->getConfig()->isDebug() ? "Leave Debug mode" : "Enter Debug mode")){
        emulator->triggerToggleDebugger();
    }

    if(Tooltip.frameCount > 0){
        ImGui::TextColored(ImVec4(1.0, 0.0, 1.0, 1.0), "%s", Tooltip.text.c_str());
        Tooltip.frameCount--;
    }

    ImGui::EndMainMenuBar();
}


/*
 *  Draws common items within the emu windows,
 *  i.e. popups which should stay open throughout the interface
 */
void Display::drawCommon() {
    if(WindowFlags.reloadROM){
        if(!ImGui::IsPopupOpen("Reload ROM?")){
            ImGui::OpenPopup("Reload ROM?");
        }

        if(ImGui::BeginPopupModal("Reload ROM?", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)){
            ImGui::Text("Do you really want to reload the ROM?");

            if(ImGui::Button("Yes")) {
                emulator->triggerReload();
                WindowFlags.reloadROM = false;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - 23);
            if(ImGui::Button("No")) {
                WindowFlags.reloadROM = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

    }


    if(WindowFlags.showDemoWindow) ImGui::ShowDemoWindow(&WindowFlags.showDemoWindow);
    if(WindowFlags.showGraphicsConfig){
        ImGui::Begin("Graphics", &WindowFlags.showGraphicsConfig, ImGuiWindowFlags_AlwaysAutoResize);
        ImGui::SliderInt("Scale", &this->newScale, 1, 6);

        float colors[4][3];
        for(size_t i = 0; i < 4; i++){
            colors[i][0] = gameboyColors[i].r / 255.0;
            colors[i][1] = gameboyColors[i].g / 255.0;
            colors[i][2] = gameboyColors[i].b / 255.0;
        }

        ImGui::Separator();
        ImGui::Text("Gameboy Pallete");

        //  Terrible, I know
        if(ImGui::ColorEdit3("Color 0", colors[0])){
            gameboyColors[0].r = colors[0][0] * 255.0;
            gameboyColors[0].g = colors[0][1] * 255.0;
            gameboyColors[0].b = colors[0][2] * 255.0;
        }
        if(ImGui::ColorEdit3("Color 1", colors[1])){
            gameboyColors[1].r = colors[1][0] * 255.0;
            gameboyColors[1].g = colors[1][1] * 255.0;
            gameboyColors[1].b = colors[1][2] * 255.0;
        }
        if(ImGui::ColorEdit3("Color 2", colors[2])){
            gameboyColors[2].r = colors[2][0] * 255.0;
            gameboyColors[2].g = colors[2][1] * 255.0;
            gameboyColors[2].b = colors[2][2] * 255.0;
        }
        if(ImGui::ColorEdit3("Color 3", colors[3])){
            gameboyColors[3].r = colors[3][0] * 255.0;
            gameboyColors[3].g = colors[3][1] * 255.0;
            gameboyColors[3].b = colors[3][2] * 255.0;
        }



        ImGui::End();
    }
    if(WindowFlags.showInputConfig){
        ImGui::Begin("Input", &WindowFlags.showInputConfig);

        ImGui::BeginColumns("InputCols", 2);

        ImGui::Text("A");
        ImGui::SameLine();
        if(ImGui::Button("A")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyA;
        }

        ImGui::Text("B");
        ImGui::SameLine();
        if(ImGui::Button("B")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyB;
        }

        ImGui::Text("Sel");
        ImGui::SameLine();
        if(ImGui::Button("Sel")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeySel;
        }

        ImGui::Text("Start");
        ImGui::SameLine();
        if(ImGui::Button("Start")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyStart;
        }

        ImGui::NextColumn();

        ImGui::Text("Up");
        ImGui::SameLine();
        if(ImGui::Button("Up")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyUp;
        }

        ImGui::Text("Down");
        ImGui::SameLine();
        if(ImGui::Button("Down")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyDown;
        }

        ImGui::Text("Left");
        ImGui::SameLine();
        if(ImGui::Button("Left")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyLeft;
        }

        ImGui::Text("Right");
        ImGui::SameLine();
        if(ImGui::Button("Right")){
            WindowFlags.requestKeybindInput = true;
            KeyEntry.binding = GBerKeyBinding::KeyRight;
        }

        ImGui::EndColumns();

        if(WindowFlags.requestKeybindInput){
            ImGui::OpenPopup("Enter a key");
            ImGui::BeginPopup("Enter a key");
            ImGui::Text("Enter a key for binding %i", KeyEntry.binding);
            ImGui::EndPopup();
        }

        ImGui::End();
    }

}

/*
 *  Sets up dockspace window flags and returns them, companion function for createDefaultDockspace
 */
ImGuiDockNodeFlags Display::startDebuggerDockspace() {
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

/*
 *  Sets up the default debugger layout
 */
void Display::createDefaultDockspace(ImGuiID existingDockspace) {
    ImGui::DockBuilderRemoveNode(existingDockspace);
    ImGui::DockBuilderAddNode(existingDockspace, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(existingDockspace, ImGui::GetMainViewport()->Size);

    ImGuiID mainDockID = existingDockspace;
    ImGuiID dockTopID = ImGui::DockBuilderSplitNode(mainDockID, ImGuiDir_Up, 0.1f, nullptr, &mainDockID);
    ImGuiID dockLeftID = ImGui::DockBuilderSplitNode(mainDockID, ImGuiDir_Left, 0.292f, nullptr, &mainDockID);
    ImGuiID dockRightID = ImGui::DockBuilderSplitNode(mainDockID, ImGuiDir_Right, 0.4f, nullptr, &mainDockID);

    ImGuiID dockRightTop = ImGui::DockBuilderSplitNode(dockRightID, ImGuiDir_Up, 0.13f, nullptr, &dockRightID);

    ImGuiID dockTopLeftFlowControl = ImGui::DockBuilderSplitNode(dockTopID, ImGuiDir_Left, 0.1f, nullptr, &dockTopID);
    ImGuiID dockTopInterrupts = ImGui::DockBuilderSplitNode(dockTopID, ImGuiDir_Left, 0.2f, nullptr, &dockTopID);
    ImGuiID dockTopPPU = ImGui::DockBuilderSplitNode(dockTopID, ImGuiDir_Left, 0.3f, nullptr, &dockTopID);

    ImGui::DockBuilderDockWindow(DebuggerWindows.flowControl.getWindowName(), dockTopLeftFlowControl);
    ImGui::DockBuilderDockWindow("GB RAM", dockLeftID);
    ImGui::DockBuilderDockWindow(DebuggerWindows.stack.getWindowName(), dockRightID);
    ImGui::DockBuilderDockWindow(DebuggerWindows.cpuDebugger.getWindowName(), dockRightTop);
    ImGui::DockBuilderDockWindow(DebuggerWindows.interruptControl.getWindowName(),dockTopInterrupts);
    ImGui::DockBuilderDockWindow(DebuggerWindows.ppuViewer.getWindowName(), dockTopPPU);


    ImGui::DockBuilderFinish(existingDockspace);
}


/*
 *  Draws the contents of the ImGui debugger window
 */
void Display::drawDebuggerContents() {
    //  Common stuff
    this->drawMenuBar();
    this->drawCommon();

    //  Set up the dockspace
    ImGuiDockNodeFlags dockspaceFlags = this->startDebuggerDockspace();
    ImGuiID dockspaceID = ImGui::GetID("DebuggerDock");
    if(ImGui::DockBuilderGetNode(dockspaceID) == nullptr || WindowFlags.resetLayout){
        createDefaultDockspace(dockspaceID);
        WindowFlags.resetLayout = false;
    }
    ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);

    //  Memory editor window
    DebuggerWindows.memEdit.DrawWindow("GB RAM", emulator->getMemory()->getBaseMemoryPointer(), 0x10000);

    //  Other windows
    DebuggerWindows.cpuDebugger.update(emulator);
    DebuggerWindows.flowControl.update(emulator);
    DebuggerWindows.ioViewer.update(emulator);
    DebuggerWindows.ppuViewer.update(emulator);
    DebuggerWindows.stack.update(emulator);
    DebuggerWindows.vramViewer.update(emulator);
    DebuggerWindows.interruptControl.update(emulator);
    DebuggerWindows.apuViewer.update(emulator);
    DebuggerWindows.gameboyScreen.update(emulator);
    DebuggerWindows.breakpointControl.update(emulator);

    //  End dockspace
    ImGui::End();
}

/*
 *  Draws the contents of the ImGui overlay when debugger is disabled
 */
void Display::drawOverlayContents() {
    this->drawMenuBar();
    this->drawCommon();

}




/*
 *  Draws the window when the debugger is enabled
 */
void Display::drawWindowDebuggerEnabled() {
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(gberWindow);
    ImGui::NewFrame();
    {
        this->drawDebuggerContents();
    }
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    int w, h;
    SDL_GetWindowSize(gberWindow, &w, &h);
    glViewport(0, 0, w, h);

    //  Flip buffers
    SDL_GL_SwapWindow(gberWindow);
}

/*
 *  Draws the window when the debugger is disabled
 */
void Display::drawWindowGameOnly() {
    //  Clear frame
    glClearColor(gameboyColors[0].r / 255.0, gameboyColors[0].g / 255.0, gameboyColors[0].b / 255.0, gameboyColors[0].a / 255.0);
    glClear(GL_COLOR_BUFFER_BIT);

    //  Send the texture to the framebuffer and blit onto the frame
    glBindFramebuffer(GL_READ_FRAMEBUFFER, this->gameboyFramebuffer);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->gameboyTexture, 0);
    glBlitFramebuffer(0, 0, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT,
                      0, 0, GAMEBOY_SCREEN_WIDTH*scale, GAMEBOY_SCREEN_HEIGHT*scale,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    //  Draw overlay
    if(overlayOpen) {
        //  Update scaling on the following frame
        if(newScale != scale){
            SDL_SetWindowSize(gberWindow, GAMEBOY_SCREEN_WIDTH*newScale, GAMEBOY_SCREEN_HEIGHT*newScale);
            scale = newScale;
        }

        ImGui::SetCurrentContext(overlayContext);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(gberWindow);
        ImGui::NewFrame();

        this->drawOverlayContents();

        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    }

    //  Set up the viewport
    int w, h;
    SDL_GetWindowSize(gberWindow, &w, &h);
    glViewport(0, 0, w, h);

    //  Flip buffers
    SDL_GL_SwapWindow(gberWindow);
}


/*
 *  Draw updated window contents
 */
void Display::updateWindow(){
    SDL_GL_MakeCurrent(gberWindow, gberGLContext);

    //  Update the screen texture
    glBindTexture(GL_TEXTURE_2D, this->gameboyTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GAMEBOY_SCREEN_WIDTH, GAMEBOY_SCREEN_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, this->framebuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    //  Depending on whether debug mode is enabled, draw differently
    if(!emulator->getConfig()->isDebug()){
        this->drawWindowGameOnly();
    } else {
        this->drawWindowDebuggerEnabled();
    }

    //  Resize the window after the frame is done, if necessary
    if(WindowFlags.requestWindowSizeChange){
        if(emulator->getConfig()->isDebug()){
            //  Turn on docking
            auto& io = ImGui::GetIO();
            io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
            io.ConfigDockingWithShift = true;

            SDL_SetWindowResizable(gberWindow, SDL_TRUE);
            SDL_MaximizeWindow(gberWindow);
        } else {
            //  Turn off docking
            auto& io = ImGui::GetIO();
            io.ConfigFlags &= ~ImGuiConfigFlags_DockingEnable;
            io.ConfigDockingWithShift = false;

            SDL_SetWindowResizable(gberWindow, SDL_FALSE);
            SDL_RestoreWindow(gberWindow);
        }
        WindowFlags.requestWindowSizeChange = false;
    }

}




/*
 *  Clears the SDL window
 */
void Display::clearWindow(){
    SDL_GL_MakeCurrent(gberWindow, gberGLContext);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

}

/*
 *  Copy pixel data of a line from PPU framebuffer to display framebuffer
 *  WARNING! The framebuffer is upside down, to compensate for OpenGL's way of storing textures
 */
void Display::appendBufferedLine(uint32_t* lineData, size_t size, size_t line){
    assert(line >= 0 && line <= GAMEBOY_SCREEN_HEIGHT);
    assert((GAMEBOY_SCREEN_HEIGHT*GAMEBOY_SCREEN_WIDTH - (line+1)*GAMEBOY_SCREEN_WIDTH) < GAMEBOY_SCREEN_WIDTH*GAMEBOY_SCREEN_HEIGHT);
    memcpy(&framebuffer[GAMEBOY_SCREEN_HEIGHT*GAMEBOY_SCREEN_WIDTH - (line+1)*GAMEBOY_SCREEN_WIDTH], lineData, size*sizeof(uint32_t));
}

/*
 *  Reloads the display module
 */
void Display::reload(){
    clearWindow();
}

/*
 *  Returns RGB color specified by gameboy color n
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

/*
 *  Handles SDL key down and key up events
 */
void Display::handleKeyEvents(SDL_Event* event) {
    auto io = ImGui::GetIO();
    auto config = emulator->getConfig();
    //  If ImGui takes over the keyboard, ignore events
    if(io.WantCaptureKeyboard) return;

    //  If we want to read a keybinding, ignore normal event handling and set the key that was pressed
    if(WindowFlags.requestKeybindInput){
        WindowFlags.requestKeybindInput = false;
        config->setKeyBinding(KeyEntry.binding, event->key.keysym.sym);
        return;
    }

    //  Better input behavior
    static bool leftHold = false,
                upHold = false,
                downHold = false,
                rightHold = false;

    //  Key up events
    if(event->type == SDL_KEYUP){
        if(event->key.keysym.sym == config->getKeyBinding(KeyA)) joypadState.aP = false;
        if(event->key.keysym.sym == config->getKeyBinding(KeyB)) joypadState.bP = false;
        if(event->key.keysym.sym == config->getKeyBinding(KeySel)) joypadState.selP = false;
        if(event->key.keysym.sym == config->getKeyBinding(KeyStart)) joypadState.startP = false;
        if(event->key.keysym.sym == config->getKeyBinding(KeyUp)) {
            joypadState.uP = false;
            upHold = false;
            //  Restore previous input
            if(downHold){
                joypadState.dP = true;
            }
        }
        if(event->key.keysym.sym == config->getKeyBinding(KeyDown)) {
            joypadState.dP = false;
            downHold = false;
            //  Restore previous input
            if(upHold){
                joypadState.uP = true;
            }
        }
        if(event->key.keysym.sym == config->getKeyBinding(KeyLeft)) {
            joypadState.lP = false;
            leftHold = false;
            //  Restore previous input
            if(rightHold){
                joypadState.rP = true;
            }
        }
        if(event->key.keysym.sym == config->getKeyBinding(KeyRight)) {
            joypadState.rP = false;
            rightHold = false;
            //  Restore previous input
            if(leftHold){
                joypadState.lP= true;
            }
        }

    } else if(event-> type == SDL_KEYDOWN){ //  Key down events
        auto ram = emulator->getMemory()->getBaseMemoryPointer();
        //  A button
        if(event->key.keysym.sym == config->getKeyBinding(KeyA)){
            joypadState.aP = true;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  B button
        if(event->key.keysym.sym == config->getKeyBinding(KeyB)){
            joypadState.bP = true;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Select button
        if(event->key.keysym.sym == config->getKeyBinding(KeySel)){
            joypadState.selP = true;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Start button
        if(event->key.keysym.sym == config->getKeyBinding(KeyStart)){
            joypadState.startP = true;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Up arrow
        if(event->key.keysym.sym == config->getKeyBinding(KeyUp)){
            upHold = true;
            joypadState.uP = true;
            if(joypadState.dP) joypadState.dP = false;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Down arrow
        if(event->key.keysym.sym == config->getKeyBinding(KeyDown)){
            downHold = true;
            joypadState.dP = true;
            if(joypadState.uP) joypadState.uP = false;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Left arrow
        if(event->key.keysym.sym == config->getKeyBinding(KeyLeft)){
            leftHold = true;
            joypadState.lP = true;
            if(joypadState.rP) joypadState.rP = false;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Right arrow
        if(event->key.keysym.sym == config->getKeyBinding(KeyRight)){
            rightHold = true;
            joypadState.rP = true;
            if(joypadState.lP) joypadState.lP = false;
            emulator->getCPU()->wakeFromStop();
            ram[IF] |= IF_HILO;
        }
        //  Overlay hotkey
        //  TODO: Add key binding for overlay
        if(event->key.keysym.sym == SDLK_TAB){
            if(!emulator->getConfig()->isDebug() && event->key.keysym.mod & KMOD_LSHIFT ){
                overlayOpen = !overlayOpen;
            }
        }

        //  Step key
        if(event->key.keysym.sym == SDLK_F3){
            if(emulator->getConfig()->isDebug()){
                emulator->getCPU()->setStep(true);
            }
        }
    }
}


/*
 *  Handles a given SDL event
 */
void Display::handleEvent(SDL_Event* event) {
    ImGui::SetCurrentContext(overlayContext);
    ImGui_ImplSDL2_ProcessEvent(event);

    switch(event->type) {
        case SDL_WINDOWEVENT_CLOSE:
        case SDL_QUIT:
            emulator->halt();
            break;
        case SDL_KEYUP:
        case SDL_KEYDOWN:
            handleKeyEvents(event);
            break;
        case SDL_DROPFILE:
            char* dropped;
            dropped = event->drop.file;
            emulator->requestChangeROM(dropped);
            SDL_free(dropped);
            break;
        default: {
            break;
        }
    }
}


/*
 *  Called when the state of the debugger changes
 */
void Display::eventDebugStatusSwitched() {
    this->WindowFlags.requestWindowSizeChange = true;
}

/*
 *  Creates a debug tooltip in the main menu bar
 */
void Display::createDebugTooltip(const std::string text, unsigned int frames) {
    if(this->Tooltip.frameCount == 0){
        this->Tooltip.frameCount = frames;
        this->Tooltip.text = text;
    }
}

/*
 *  Returns the GLid of the gameboy screen texture
 */
GLuint Display::getScreenTexture() {
    return gameboyTexture;
}

int Display::getScale() {
    return scale;
}

std::chrono::milliseconds Display::updateFrameTime() {
    static std::chrono::time_point previous = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - previous);
    previous = std::chrono::system_clock::now();

    return duration;
}
