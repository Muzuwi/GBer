#pragma once
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"
#include "Headers/PPU.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Display.hpp"

class DebugPPU{
    const std::string windowName = "PPU Status";

public:
    void updateWindow(ImGuiIO* io, PPU* ppu, RAM* ram, Display* display){
        ImGui::Begin(windowName.c_str());
        {

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
        ImGui::End();

    }



};