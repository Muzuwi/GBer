#pragma once
#include <fstream>
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"
#include "Headers/LR35902.hpp"
#include "Headers/RAM.hpp"

class DebugStack{
    const std::string windowName = "Stack";

public:
    void updateWindow(ImGuiIO* io, LR35902* cpu, RAM* ram){
        ImGui::Begin(windowName.c_str());
        {
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
        ImGui::End();

    }
};