#pragma once
#include <string>
#include <Headers/RAM.hpp>
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"

class DebugIO{
    const std::string windowName = "IO Viewer";
public:
    bool open = false;

    void updateWindow(ImGuiIO* io, LR35902* cpu, RAM* ram){
        std::string tempstring;
        if(!open) return;

        ImGui::Begin(windowName.c_str(), &this->open);

//        ImGui::Text(("T-Freq: " + std::to_string(cpu->getTimerFrequency())).c_str());
//        tempstring = "Timer status: ";
//        tempstring += ((ram->peek(TAC) & 0x4) ? " enabled" : " disabled");

        ImGui::Columns(6, NULL, false);

        ImGui::Text("General");
#define REGISTER_EDIT(a) ImGui::PushItemWidth(21.0); \
                         ImGui::InputScalar(#a, ImGuiDataType_U8, &ram->getBaseMemoryPointer()[a], NULL, NULL, "%02X", ImGuiInputTextFlags_CharsHexadecimal);
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

        ImGui::End();
    }

};