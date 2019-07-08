#pragma once
#include "imgui/imgui.h"
#include "Headers/LR35902.hpp"
#include "Headers/Display.hpp"

class DebugFlow{
    const std::string windowName = "Flow Control";
    bool showFlow = true, pauseDebugger = false;

public:
    bool shouldPauseDebugger(){
        return pauseDebugger;
    }

    void clearShouldPauseDebugger(){
        pauseDebugger = false;
    }

    bool updateWindow(ImGuiIO* io, LR35902* cpu, Display* display){
        //  HACKY HACK
        bool ret = false;

        ImGui::Begin(windowName.c_str(), &showFlow);
        if(ImGui::ArrowButton("Continue Execution", ImGuiDir_Right)){
            cpu->setStep(true);
            cpu->setContinueExec(true);

            SDL_RaiseWindow(display->getWindow());
            pauseDebugger = true;
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
            ret = true;
        }


        if(cpu->isCPUHalted()){
            ImGui::Text("HALT");
        }

        ImGui::End();

        return ret;
    }
};
