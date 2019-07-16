#pragma once
#include <string>
#include "imgui/imgui.h"
#include "Headers/RAM.hpp"

class DebugAPU{
    const std::string windowName = "APU";

public:
    bool open = false;

    void updateWindow(ImGuiIO* io, RAM* ram){
        if(!open) return;
        ImGui::Begin(windowName.c_str());

        ImGui::Text("Channel 1");


        ImGui::Text("Channel 2");



        ImGui::Text("Channel 3");


        ImGui::Text("Channel 4");



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

        ImGui::End();
    }

};