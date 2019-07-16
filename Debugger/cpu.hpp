#pragma once
#include <string>
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"
#include "Headers/Structures.hpp"

class DebugCPU{
    const std::string windowName = "CPU";
    bool checkZ, checkN, checkH, checkC;

public:
    inline uint16_t readAF(Reg Registers){
        return Registers.A * 0x100 + Registers.F;
    }

    inline uint16_t readHL(Reg Registers){
        return Registers.H * 0x100 + Registers.L;
    }

    inline uint16_t readBC(Reg Registers){
        return Registers.B * 0x100 + Registers.C;
    }

    inline uint16_t readDE(Reg Registers){
        return Registers.D * 0x100 + Registers.E;
    }

    void updateWindow(ImGuiIO* io, Reg registers){
        ImGui::Begin(windowName.c_str(), NULL);
        {
            std::string temp;

            temp = "AF: " + Utils::decHex(readAF(registers));
            ImGui::Text(temp.c_str());
            ImGui::SameLine();
            temp = "BC: " + Utils::decHex(readBC(registers));
            ImGui::Text(temp.c_str());

            temp = "DE: " + Utils::decHex(readDE(registers));
            ImGui::Text(temp.c_str());
            ImGui::SameLine();
            temp = "HL: " + Utils::decHex(readHL(registers));
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
        ImGui::End();
    }
};