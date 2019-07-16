#pragma once
#include <string>
#include "imgui/imgui.h"
#include "Headers/Utils.hpp"
#include "Headers/Structures.hpp"

class DebugBreakpoint{
    const std::string windowName = "Breakpoints";

    //  Breakpoints
    std::vector<uint16_t> addressBreakpoints;
    std::vector<uint8_t> instructionBreakpoints;
    std::vector<MemoryBreakpoint> memoryBreakpoints;

    bool showVal = false, read = false, write = false, prefixCB = false;

public:
    bool open = false;

    void updateWindow(ImGuiIO* io){
        if(!open) return;
        ImGui::Begin(windowName.c_str(), &this->open);
        {
            static char buffer[6] = "";
            int memoryBreakpointValue = 0;

            ImGui::PushItemWidth(100);
            ImGui::InputText("Address", buffer, IM_ARRAYSIZE(buffer));
            ImGui::SameLine();
            ImGui::Checkbox("Val", &showVal);
            if(showVal){
                ImGui::SameLine();
                ImGui::PushItemWidth(120);
                ImGui::InputInt("Value", &memoryBreakpointValue, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
            }
            ImGui::Checkbox("R##read", &read);
            ImGui::SameLine();
            ImGui::Checkbox("W##write", &write);
            ImGui::SameLine();
            ImGui::Checkbox("Prefix CB", &prefixCB);

            ImGui::SameLine(0.0f, 30.0f);
            if(ImGui::Button("Add")){
                if(std::string("").compare(buffer)){
                    int casted = strtol(buffer, NULL, 16);
                    if(!(casted < 0 || casted > 0xFFFF)){
                        addAddressBreakpoint(casted);
                    }
                }
            }

            ImGui::SameLine();
            if(ImGui::Button("Add brOP")){
                if(std::string("").compare(buffer)){
                    int casted = strtol(buffer, NULL, 16);
                    if(!(casted <= 0 || casted > 0xFF)){
                        addOpBreakpoint(casted);
                    }
                }
            }

            ImGui::SameLine();
            if(ImGui::Button("Add memBr")){
                if(std::string("").compare(buffer) && (read || write)){
                    int casted = strtol(buffer, NULL, 16);
                    if(!(casted <= 0 || casted > 0xFFFF)){
                        addMemoryBreakpoint(casted, read, write, showVal, memoryBreakpointValue);
                    }
                }
            }

            //   Breakpoints
            ImGui::BeginChild("breakpointlist");
            ImGui::Columns(3);

            //  Address breakpoints
            if(ImGui::Button("Rmv. All##bk")){
                addressBreakpoints.clear();
            }
            int id = 0, buttonID = 0;
            for(char16_t a : addressBreakpoints){
                ImGui::PushID(buttonID);
                ImGui::BulletText(Utils::decHex(a).c_str());
                ImGui::SameLine();
                std::string tag = "x##bp." + Utils::decHex(a);
                if(ImGui::SmallButton(tag.c_str())){
                    addressBreakpoints.erase(addressBreakpoints.begin()+id);
                }
                ++id;
                ImGui::PopID();
                ++buttonID;
            }

            //  OP Breakpoints
            ImGui::NextColumn();
            if(ImGui::Button("Rmv. All##bop")){
                instructionBreakpoints.clear();
            }
            int id2 = 0;
            for(uint8_t a : instructionBreakpoints){
                ImGui::PushID(buttonID);
                ImGui::BulletText(Utils::decHex(a, 2).c_str());
                ImGui::SameLine();
                std::string tag = "x##bOP." + Utils::decHex(a);
                if(ImGui::SmallButton(tag.c_str())){
                    instructionBreakpoints.erase(instructionBreakpoints.begin()+id2);
                }
                ++id2;
                ImGui::PopID();
                ++buttonID;
            }

            //  Memory breakpoints
            ImGui::NextColumn();
            if(ImGui::Button("Rmv. All##bpMr")){
                memoryBreakpoints.clear();
            }
            int id3 = 0;
            for(MemoryBreakpoint br : memoryBreakpoints){
                ImGui::PushID(buttonID);
                std::string label = Utils::decHex(br.addr) + " [";
                if(br.r) label += "R";
                if(br.w) label += "W";
                if(br.v) label += "V, $" + Utils::decHex(br.val,2);
                label += "]";
                ImGui::BulletText(label.c_str());
                ImGui::SameLine();
                std::string tag = "x##bpM." + Utils::decHex(br.addr);
                if(ImGui::SmallButton(tag.c_str())){
                    memoryBreakpoints.erase(memoryBreakpoints.begin()+id3);
                }
                ++id3;
                ImGui::PopID();
                ++buttonID;
            }
            ImGui::EndChild();
        }
        ImGui::End();

    }

    void addAddressBreakpoint(uint16_t address){
        addressBreakpoints.push_back(address);
    }

    void addOpBreakpoint(uint8_t op){
        instructionBreakpoints.push_back(op);
    }

    void addMemoryBreakpoint(uint16_t address, bool read, bool write, bool val, uint8_t expectedValue){
        memoryBreakpoints.push_back(MemoryBreakpoint(address, read, write, val, expectedValue));
    }

    void addMemoryBreakpoint(MemoryBreakpoint breakpoint){
        memoryBreakpoints.push_back(breakpoint);
    }

    bool checkAddressBreakpoint(uint16_t address){
        for(uint16_t addr : addressBreakpoints){
            if(addr == address) return true;
        }
        return false;
    }

    bool checkInstructionBreakpoint(uint8_t op){
        for(uint8_t opcode : instructionBreakpoints){
            if(op == opcode) return true;
        }
        return false;
    }

    bool checkMemoryBreakpoint(uint16_t address){
        for(MemoryBreakpoint breakpoint : memoryBreakpoints){
            if(breakpoint.addr == address && breakpoint.r) return true;
        }
        return false;
    }

    bool checkMemoryBreakpoint(uint16_t address, uint8_t byte){
        for(MemoryBreakpoint breakpoint : memoryBreakpoints){
            if(breakpoint.addr == address && breakpoint.w){
                if(breakpoint.v){
                    if(breakpoint.val == byte) return true;
                } else {
                    return true;
                }
            }
        }
        return false;
    }

};