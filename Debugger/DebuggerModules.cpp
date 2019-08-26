#include "DebuggerModules.hpp"
#include "Headers/Emulator.hpp"

/*
 *  This file contains the definitions for all of the debugger widgets
 *  Virtual function updateWindowContents is called when a frame is drawn
 *  and after ImGui::Begin(windowName) was called
 */

/*
 *  CPU debugger window (registers)
 */
void DebugCPU::updateWindowContents(Emulator *emulator) {
    auto registers = emulator->getCPU()->getRegisters();
    std::string temp;
    temp = "AF: " + Utils::decHex((registers.A << 8) | registers.F);
    ImGui::Text(temp.c_str());
    ImGui::SameLine();
    temp = "BC: " + Utils::decHex((registers.B << 8) | registers.C);
    ImGui::Text(temp.c_str());

    temp = "DE: " + Utils::decHex((registers.D << 8) | registers.E);
    ImGui::Text(temp.c_str());
    ImGui::SameLine();
    temp = "HL: " + Utils::decHex((registers.H << 8) | registers.L);
    ImGui::Text(temp.c_str());

    temp = "SP: " + Utils::decHex(registers.SP);
    ImGui::Text(temp.c_str());
    ImGui::SameLine();
    temp = "PC: " + Utils::decHex(registers.PC);
    ImGui::Text(temp.c_str());

    checkZ = (registers.F >> 7);
    checkN =  ((registers.F & 0x40) >> 6);
    checkH = ((registers.F & 0x20) >> 5);
    checkC = ((registers.F & 0x10) >> 4);

    ImGui::Checkbox("Z", &checkZ);
    ImGui::SameLine();
    ImGui::Checkbox("N", &checkN);
    ImGui::SameLine();
    ImGui::Checkbox("H", &checkH);
    ImGui::SameLine();
    ImGui::Checkbox("C", &checkC);
}


/*
 *  Flow control window (step, continue, ...)
 */
void DebugFlow::updateWindowContents(Emulator *emulator) {
    auto cpu = emulator->getCPU();

    if(ImGui::ArrowButton("Continue Execution", ImGuiDir_Right)){
        cpu->setStep(true);
        cpu->setContinueExec(true);

    }
    ImGui::SameLine();
    ImGui::Text("Continue");
    if(ImGui::ArrowButton("Trace", ImGuiDir_Down)){
        cpu->setContinueExec(false);
        cpu->setStep(true);
    }
    ImGui::SameLine();
    ImGui::Text("Trace Into");
    if(ImGui::Button("Reload")){
        emulator->triggerReload();
    }


    if(cpu->isCPUHalted()){
        ImGui::Text("HALT");
    }
}


/*
 *  Current interrupts
 */
void DebugInterrupts::updateWindowContents(Emulator *emulator) {
    auto ram = emulator->getMemory();

    std::string iemString = "IME: ";
    iemString += ( (emulator->getCPU()->getRegisters().IME) ? "enabled" : "disabled");
    ImGui::Text(iemString.c_str());
    std::string ieString = "Enabled ints: ";
    if(ram->peek(0xFFFF) & 1) ieString += "VBlank ";
    if(ram->peek(0xFFFF) & 2 ) ieString += "LCDC ";
    if(ram->peek(0xFFFF) & 4 ) ieString += "Timer ";
    if(ram->peek(0xFFFF) & 8 ) ieString += "Ser I/O ";
    if(ram->peek(0xFFFF) & 0x10 ) ieString += "P10-P13 ";
    ImGui::Text(ieString.c_str());
}


/*
 *  I/O Viewer
 */
void DebugIO::updateWindowContents(Emulator *emulator) {
    auto ram = emulator->getMemory();
    ImGui::Columns(6, NULL, false);
    //  Register editor macro, takes a macro defined address as an argument
    #define REGISTER_EDIT(a) ImGui::PushItemWidth(21.0); \
                             ImGui::InputScalar(#a, ImGuiDataType_U8, &ram->getBaseMemoryPointer()[a], NULL, NULL, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
    ImGui::Text("General");
    REGISTER_EDIT(SB)
    REGISTER_EDIT(SC)
    REGISTER_EDIT(DIV)
    REGISTER_EDIT(TIMA)
    REGISTER_EDIT(TMA)
    REGISTER_EDIT(TAC)
    REGISTER_EDIT(IE)
    REGISTER_EDIT(IF)
    REGISTER_EDIT(P1)

    ImGui::NextColumn();

    ImGui::Text("PPU");
    REGISTER_EDIT(LCDC)
    REGISTER_EDIT(STAT)
    REGISTER_EDIT(DMA)
    REGISTER_EDIT(SCX)
    REGISTER_EDIT(SCY)
    REGISTER_EDIT(WX)
    REGISTER_EDIT(WY)
    REGISTER_EDIT(LY)
    REGISTER_EDIT(LYC)
    REGISTER_EDIT(BGP)
    REGISTER_EDIT(OBP0)
    REGISTER_EDIT(OBP1)

    ImGui::NextColumn();
    ImGui::Text("Sound 1");
    REGISTER_EDIT(NR10)
    REGISTER_EDIT(NR11)
    REGISTER_EDIT(NR12)
    REGISTER_EDIT(NR13)
    REGISTER_EDIT(NR14)

    ImGui::NextColumn();
    ImGui::Text("Sound 2");

    REGISTER_EDIT(NR21)
    REGISTER_EDIT(NR22)
    REGISTER_EDIT(NR23)
    REGISTER_EDIT(NR24)

    ImGui::NextColumn();
    ImGui::Text("Sound 3");

    REGISTER_EDIT(NR30)
    REGISTER_EDIT(NR31)
    REGISTER_EDIT(NR32)
    REGISTER_EDIT(NR33)
    REGISTER_EDIT(NR34)

    ImGui::NextColumn();
    ImGui::Text("Sound 4");
    REGISTER_EDIT(NR41)
    REGISTER_EDIT(NR42)
    REGISTER_EDIT(NR43)
    REGISTER_EDIT(NR44)


    ImGui::Text("Sound Ctrl.");
    REGISTER_EDIT(NR50)
    REGISTER_EDIT(NR51)
    REGISTER_EDIT(NR52)
}


/*
 *  APU control
 */
void DebugAPU::updateWindowContents(Emulator *emulator) {
    auto ram = emulator->getMemory();

    ImGui::Text("Channel 1");
    auto reg = emulator->getAPU()->getSM1();
    ImGui::Text("Frequency: %i", reg->frequency);
    switch(reg->dutyRegister.wavePatternDuty){
        case Duty::Duty_12_5:{
            const float samples[] {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
            ImGui::PlotHistogram("Duty cycle: 12.5%", samples, IM_ARRAYSIZE(samples), 0, nullptr, 0, 1, ImVec2(100, 20));
            break;
        }
        case Duty::Duty_25:{
            const float samples[] {1.0f, 0.0f, 0.0f, 0.0f};
            ImGui::PlotHistogram("Duty cycle: 25%", samples, IM_ARRAYSIZE(samples), 0, nullptr, 0, 1, ImVec2(100, 20));
            break;
        }
        case Duty::Duty_50:{
            const float samples[] {1.0f, 1.0f, 0.0f, 0.0f};
            ImGui::PlotHistogram("Duty cycle: 50%", samples, IM_ARRAYSIZE(samples), 0, nullptr, 0, 1, ImVec2(100, 20));
            break;
        }
        case Duty::Duty_75:{
            const float samples[] {1.0f, 1.0f, 1.0f, 0.0f};
            ImGui::PlotHistogram("Duty cycle: 75%", samples, IM_ARRAYSIZE(samples), 0, nullptr, 0, 1, ImVec2(100, 20));
            break;
        }
    }
    ImGui::Checkbox("Repeat", &reg->repeat);


    ImGui::Separator();
    ImGui::Text("Channel 2");

    ImGui::Separator();
    ImGui::Text("Channel 3");

    ImGui::Separator();
    ImGui::Text("Channel 4");

    ImGui::Separator();
    ImGui::Text("Control Channel");
    bool SO1[4], SO2[4];
    for(unsigned int i = 0; i < 4; i++ ) SO1[i] = ram->peek(NR51) & (1 << i);
    for(unsigned int i = 4; i < 8; i++ ) SO2[i-4] = ram->peek(NR51) & (1 << i);
    ImGui::Text("L");
    for(size_t i = 0; i < 4; i++){
        ImGui::SameLine();
        ImGui::Checkbox(std::to_string(i).c_str(), &SO1[i]);
    }
    ImGui::Text("R");
    for(size_t i = 0; i < 4; i++){
        ImGui::SameLine();
        ImGui::Checkbox(std::to_string(i).c_str(), &SO2[i]);
    }
}


/*
 *  Stack viewer
 */
void DebugStack::updateWindowContents(Emulator *emulator) {
    auto cpu = emulator->getCPU();
    auto ram = emulator->getMemory();

    for(int i = 0xFFFE; i >= cpu->getRegisters().SP; i -= 2){
        std::string label = Utils::decHex(i-1) + ": " + Utils::decHex(ram->peek(i)*0x100 + ram->peek(i-1));
        ImGui::Text(label.c_str());
    }
    if(ImGui::Button("Save")){
        std::fstream file;
        file.open("stackdump.bin", std::ios::binary);
        int siz = 0xFFFF - cpu->getRegisters().SP;
        auto pointer = ram->getBaseMemoryPointer();
        file.write((char*)&pointer[cpu->getRegisters().SP], siz * sizeof(unsigned char));
        file.close();
    }
    ImGui::SetScrollHere(1.0f);
}


/*
 *  PPU Status viewer
 */
void DebugPPU::updateWindowContents(Emulator *emulator) {
    auto ppu = emulator->getPPU();
    auto display = emulator->getDisplay();

    std::string mode;
    switch(ppu->getPPUMode()){
        case OAM:
            mode = "OAM";
            break;
        case VBLANK:
            mode = "VBlank";
            break;
        case HBLANK:
            mode = "HBlank";
            break;
        case PIXTX:
            mode = "Pixel Transfer";
            break;
    }
    ImGui::Text(("PPU Mode: " + mode).c_str());
    ImGui::Text(("PPU cycles: " + std::to_string(ppu->getPPUCycles())).c_str());
    ImGui::Text("LCDC Status: ");
    ImGui::SameLine();
    ImGui::Checkbox("lcdEn", &ppu->getLCDC()->lcdEnable);
    ImGui::SameLine();
    ImGui::Checkbox("objEn", &ppu->getLCDC()->objEnable);
    ImGui::SameLine();
    ImGui::Checkbox("wndEn", &ppu->getLCDC()->windowEnable);
    ImGui::SameLine();
    ImGui::Checkbox("priority", &ppu->getLCDC()->bgWindowDisplayPriority);

    std::string tempstring;
    tempstring = "bgMap: $" + Utils::decHex(ppu->getLCDC()->bgTileMap.lower) + "-$" + Utils::decHex(ppu->getLCDC()->bgTileMap.higher);
    ImGui::Text(tempstring.c_str());
    tempstring = "bgWndTileData: $" + Utils::decHex(ppu->getLCDC()->bgWindowTileData.lower) + "-$" + Utils::decHex(ppu->getLCDC()->bgWindowTileData.higher);
    ImGui::Text(tempstring.c_str());
    tempstring = "wndTileMap: $" + Utils::decHex(ppu->getLCDC()->windowTileMap.lower) + "-$" + Utils::decHex(ppu->getLCDC()->windowTileMap.higher);
    ImGui::Text(tempstring.c_str());
    tempstring = "objSize: 8x" + std::to_string(ppu->getLCDC()->objHeight);
    ImGui::Text(tempstring.c_str());

    ImGui::Dummy(ImVec2(18.0f, 0.0f));
    ImGui::SameLine();
    ImGui::Checkbox("U", &display->getJoypad()->uP);
    ImGui::Checkbox("L", &display->getJoypad()->lP);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(7.0f, 0.0f));
    ImGui::SameLine();
    ImGui::Checkbox("R", &display->getJoypad()->rP);

    ImGui::Dummy(ImVec2(18.0f, 0.0f));
    ImGui::SameLine();
    ImGui::Checkbox("D", &display->getJoypad()->dP);

    ImGui::Checkbox("A", &display->getJoypad()->aP);
    ImGui::Checkbox("B", &display->getJoypad()->bP);
    ImGui::Checkbox("Sel", &display->getJoypad()->selP);
    ImGui::Checkbox("Start", &display->getJoypad()->startP);
}


/*
 *  VRAM tile viewer
 */
void DebugVRAM::updateWindowContents(Emulator *emulator) {
    if(ImGui::BeginTabBar("##vramtabs", ImGuiTabBarFlags_None)){
        if(ImGui::BeginTabItem("Tile Data", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            updateTileData(emulator);
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Background Map", nullptr, ImGuiWindowFlags_AlwaysAutoResize)){
            updateMap(emulator);
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
//  Draws the map
void DebugVRAM::updateMap(Emulator *emulator) {
    auto ram = emulator->getMemory();
    auto ppu = emulator->getPPU();

    for(int i = 0; i < (ppu->getLCDC()->bgTileMap.higher - ppu->getLCDC()->bgTileMap.lower); i++){
        uint8_t tileID = ram->peek(ppu->getLCDC()->bgTileMap.lower + i);
        unsigned int posX = i%0x20,
                posY = i/0x20;

        for(int j = 0; j < 8; j++){
            //  Get raw tile data at line j
            uint8_t tileBuffer[8];
            ppu->debugGetRawTileDataWrapper(tileID, j, tileBuffer, 8, 0, 8, false, false, 0);

            //  Add pixels to the buffer
            for(int pxl = 0; pxl < 8; pxl++){
                unsigned int arrayPosY = posY*8 + j,
                        arrayPosX = posX*8 + pxl;

                uint32_t pixel = 0;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].r) << 24;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].g) << 16;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].b) << 8;
                pixel |= 0xFF;

                backgroundMapBuffer[arrayPosY*backgroundMapWidth + arrayPosX] = pixel;
            }
        }


    }
    //  Draw viewport
    for(int i = ram->peek(SCX); i < 160+ram->peek(SCX); i++){
        if(i == ram->peek(SCX) || i == 160+ram->peek(SCX)-1){
            for(int j = ram->peek(SCY); j < ram->peek(SCY) + 144; j++){
                unsigned int y = j % (0x20*8);
                backgroundMapBuffer[y*backgroundMapWidth + i] = 0xFF0000FF;
            }
        }
        unsigned int x = i % (0x20*8);
        //  Draw top
        backgroundMapBuffer[ram->peek(SCY)*backgroundMapWidth + x] = 0xFF0000FF;
        //  Draw bottom
        backgroundMapBuffer[((ram->peek(SCY)+144)%backgroundMapHeight)*backgroundMapWidth + x] = 0xFF0000FF;
    }

    //  Update GL texture
    glBindTexture(GL_TEXTURE_2D, backgroundMapTextureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backgroundMapWidth, backgroundMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backgroundMapBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    ImGui::Image((void*)backgroundMapTextureID, ImVec2(0x20*8, 0x20*8));
}
//  Draws tile data from RAM
void DebugVRAM::updateTileData(Emulator *emulator) {
    auto ppu = emulator->getPPU();
    auto& io = ImGui::GetIO();

    ImGui::Checkbox("XFlip", &flipTilesX);
    ImGui::SameLine();
    ImGui::Checkbox("YFlip", &flipTilesY);

    //  Draw nice tiles
    for(int i = 0; i < (0x97F0 - 0x8000)/0x10 + 1; i++){
        unsigned int posX = i%0x10,
                posY = i/0x10;
        for(int line = 0; line < 8; line++){
            //  Get tile data
            uint8_t tileBuffer[8];

            if(i < 0x100){
                ppu->debugGetRawTileDataWrapper(i, line, tileBuffer, 8, 0, 8, flipTilesX, flipTilesY, 0x8000);
            } else {
                ppu->debugGetRawTileDataWrapper(i-0x100, line, tileBuffer, 8, 0, 8, flipTilesX, flipTilesY, 0x9000);
            }

            for(int pxl = 0; pxl < 8; pxl++){
                unsigned int arrayPosY = posY*8 + line,
                        arrayPosX = posX*8 + pxl;

                uint32_t pixel = 0;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].r) << 24;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].g) << 16;
                pixel |= ((uint32_t)colors[tileBuffer[pxl]].b) << 8;
                pixel |= 0xFF;

                assert(arrayPosY*tileMapWidth + arrayPosX <= tileMapHeight*tileMapWidth);
                tileMapBuffer[arrayPosY*tileMapWidth + arrayPosX] = pixel;

            }
        }
    }

    //  Update texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tileMapWidth, tileMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tileMapBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    const unsigned int scale = 2;

    //  Draw tile preview on mouse over
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGui::Image((void*)textureID, ImVec2(tileMapWidth*scale, tileMapHeight*scale));
    if(ImGui::IsItemHovered()){
        ImGui::BeginTooltip();
        float size = 64.0f;
        float tileX = (int)(io.MousePos.x - pos.x) / (8*scale), tileY = (int)(io.MousePos.y - pos.y) / (8*scale);
        ImVec2 topLeft = ImVec2(tileX*8,tileY*8),
                uv0 = ImVec2(topLeft.x / tileMapWidth, topLeft.y / tileMapHeight),
                uv1 = ImVec2( (topLeft.x + 8) / tileMapWidth, (topLeft.y + 8) / tileMapHeight);
        ImGui::Image((void*)textureID, ImVec2(size, size), uv0, uv1);

        uint8_t tileID = (((int)tileY % 0x10) << 4) | (int)tileX;
        ImGui::Text("Tile ID: %02x", tileID);
        uint16_t address = tileY < 0x10 ? 0x8000 : 0x9000;
        address += tileID << 4;
        ImGui::Text("Tile Address: %04x", address);


        ImGui::EndTooltip();
    }
}
//  Generates textures needed by the widget
void DebugVRAM::init() {
    //  Clear buffers
    std::fill_n(&this->tileMapBuffer[0], tileMapWidth*tileMapHeight, 0xFFFFFFFF);
    std::fill_n(&this->backgroundMapBuffer[0], backgroundMapWidth*backgroundMapHeight, 0xFFFFFFFF);

    //  Generate GL textures
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tileMapWidth, tileMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tileMapBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &backgroundMapTextureID);
    glBindTexture(GL_TEXTURE_2D, backgroundMapTextureID);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0x20*8, 0x20*8, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backgroundMapBuffer);
    glBindTexture(GL_TEXTURE_2D, 0);

    //  Set up colors
    colors[0].r = colors[0].g = colors[0].b = 255;
    colors[1].r = colors[1].g = colors[1].b = 170;
    colors[2].r = colors[2].g = colors[2].b = 85;
    colors[3].r = colors[3].g = colors[3].b = 0;
}


/*
 *  Breakpoint control
 */
void DebugBreakpoints::updateWindowContents(Emulator *emulator) {
    //  TODO:
    static bool showVal = false, read = false, write = false, execute = false, prefixCB = false;
    static char buffer[6] = "";
    static uint8_t value = 0;

    ImGui::PushItemWidth(100);
    ImGui::InputText("Address/OP", buffer, IM_ARRAYSIZE(buffer));
    ImGui::SameLine();
    ImGui::Checkbox("Val", &showVal);
    if(showVal){
        ImGui::SameLine();
        //ImGui::PushItemWidth(120);
        //ImGui::InputInt("Value", &memoryBreakpointValue, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
        ImGui::InputScalar("Value", ImGuiDataType_U8, &value);
    }
    ImGui::Checkbox("R##read", &read);
    ImGui::SameLine();
    ImGui::Checkbox("W##write", &write);
    ImGui::SameLine();
    ImGui::Checkbox("X##execute", &execute);
    ImGui::SameLine();
    ImGui::Checkbox("Prefix CB", &prefixCB);

    /*
     *          if(std::string("").compare(buffer)){
                    int casted = strtol(buffer, NULL, 16);
                    if(!(casted < 0 || casted > 0xFFFF)){
                        addAddressBreakpoint(casted);
                    }
                }
     */

    ImGui::SameLine();
    if(ImGui::Button("Add") && std::string("").compare(buffer)){
        unsigned int castedVal = strtol(buffer, nullptr, 16);
        MemoryBreakpoint breakpoint(castedVal, read, write, showVal, value);
        if(read || write){
            emulator->getDebugger()->addMemoryBreakpoint(breakpoint);
        }
        if(execute){
            emulator->getDebugger()->addOpBreakpoint(castedVal);
        }
    }

}

/*
 *  Gameboy screen
 */
void DebugGameboyScreen::updateWindowContents(Emulator *emulator) {
    auto scale = emulator->getDisplay()->getScale();
    ImGui::Image((void*)emulator->getDisplay()->getScreenTexture(), ImVec2(GAMEBOY_SCREEN_WIDTH*scale, GAMEBOY_SCREEN_HEIGHT*scale), ImVec2(0,1), ImVec2(1,0));
}
