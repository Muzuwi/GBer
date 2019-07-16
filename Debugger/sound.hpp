#pragma once
#include <SDL2/SDL.h>
#include <cmath>
#include <Headers/Debug.hpp>
#include "imgui/imgui.h"
#include "Headers/RAM.hpp"

class DebugSound{
    const std::string windowName = "APU";

    bool playRequested;
    float buffer[32];
    float buf[4096];
    SDL_AudioDeviceID deviceID;

public:
    void init(){
        SDL_AudioSpec requested, acquired;
        SDL_zero(requested);
        requested.freq = 44800;
        requested.format = AUDIO_U8;
        requested.channels = 2;
        requested.samples = 4096;
        requested.callback = NULL;
        deviceID = SDL_OpenAudioDevice(NULL, 0, &requested, &acquired, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
        if(deviceID == 0) Debug::emuLog("Debugger::sound/ Audio device open failed");
        else {
            std::string log = "Debugger::sound/ Obtained audio format: freq " + std::to_string(acquired.freq)
                            + ", format " + std::to_string(acquired.format) + ", channels " + std::to_string(acquired.channels)
                            + ", samples " + std::to_string(acquired.samples);
            Debug::emuLog(log);
            //std::cout << "Obtained format freq " << acquired.freq << ", format " << acquired.freq << ", channels " << acquired.channels << ", samples " << acquired.samples << "\n";
        }
        SDL_PauseAudioDevice(deviceID,0);
    }

    /*static void audioCallback(void* userData, unsigned char* stream, int len){
        if(playRequested){
            for(size_t i = 0; i < 32; i++){
                stream[i] = buffer[i];
            }
            for(size_t i = 32; i < len; i++){
                stream[i] = 0;
            }

            playRequested = false;
        } else {
            for(size_t i = 0; i < len; i++){
                stream[i] = 0;
            }
        }
    }*/

    void updateWindow(ImGuiIO* io){
        ImGui::Begin(windowName.c_str());
        {
            for(unsigned int i = 0; i < 16; i += 2){
                buffer[i] = (RAM::RAM[0xFF30 + i] & 0xF0) >> 4;
                buffer[i+1] = RAM::RAM[0xFF30 + i] & 0x0F;
            }
            ImGui::PlotLines("Wave Pattern RAM", buffer, 32);
            if(ImGui::ArrowButton("Play", ImGuiDir_Right) && deviceID != 0){
                //float volume = 0.1;
                //for(size_t i = 0; i < 4096; i++){
                //    buf[i] = volume*sin(2*3.14159*10000*(i*3.14159/8));
                //}
                //SDL_QueueAudio(deviceID, buf, 4096);
                SDL_QueueAudio(deviceID, buffer, 32);
            }
        }
        ImGui::End();
    }

};