#pragma once
#include "imgui/imgui.h"

class Emulator;

class DebuggerWindow{
    /*
    *  Override this function and add your own elements to the window
    */
    virtual void updateWindowContents(Emulator* emulator){

    }

    /*
     *  Override this function to set the window flags
     */
    virtual ImGuiWindowFlags getWindowFlags(){
        return ImGuiWindowFlags_None;
    }


public:
    bool open = true;

    /*
     *  Returns the name of the window
     */
    virtual const char* getWindowName(){
        return "Placeholder";
    }

    /*
     *  Updates the window contents
     */
    void update(Emulator* emulator){
        if(!open) {
            return;
        }

        ImGui::Begin(getWindowName(), &open, getWindowFlags());

        updateWindowContents(emulator);

        ImGui::End();
    }
};
