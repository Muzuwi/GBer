#pragma once
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"
#include "Headers/LR35902.hpp"
#include "Headers/RAM.hpp"

class DebugInterrupt{
    const std::string windowName = "Interrupts";

public:
    void updateWindow(ImGuiIO* io, LR35902* cpu, RAM* ram){
        ImGui::Begin(windowName.c_str());
        {
            std::string iemString = "IME: ";
            iemString += ( (cpu->getRegisters().IME) ? "enabled" : "disabled");
            ImGui::Text(iemString.c_str());
            std::string ieString = "Enabled ints: ";
            if(ram->peek(0xFFFF) & 1) ieString += "VBlank ";
            if(ram->peek(0xFFFF) & 2 ) ieString += "LCDC ";
            if(ram->peek(0xFFFF) & 4 ) ieString += "Timer ";
            if(ram->peek(0xFFFF) & 8 ) ieString += "Ser I/O ";
            if(ram->peek(0xFFFF) & 0x10 ) ieString += "P10-P13 ";
            ImGui::Text(ieString.c_str());
        }
        ImGui::End();
    }

};