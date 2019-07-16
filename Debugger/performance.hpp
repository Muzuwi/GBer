#pragma once
#include <chrono>
#include <imgui/imgui.h>
#include <string>

class DebugPerformance{
    std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> start, end;
    std::chrono::duration<long long int, std::nano> duration;
    float fiveSecondBuffer[5*60];
    int index = 0;

public:

    void updateWindow(ImGuiIO* io) {
        ImGui::Begin("Performance");
        float fps = 1.0 / (duration.count() / 1000000000.0);
        if(index >= 5*60) index = 0;
        fiveSecondBuffer[index++] = fps;

        //std::cout << duration.count() << " " << std::chrono::duration_cast<std::chrono::seconds>(duration).count() << "\n";
        ImGui::Text(("FPS: " + std::to_string(fps)).c_str());
        ImGui::PlotHistogram("FPS", fiveSecondBuffer, 5*60, 0, NULL, 0, 100, ImVec2(400, 80));
        ImGui::End();
    }

    void updateTimer(std::chrono::time_point<std::chrono::system_clock, std::chrono::nanoseconds> ending){
        end = ending;
        duration = (end - start);
        start = end;
    }

    std::chrono::duration<long long int, std::nano> getLastFrameDuration(){
        return duration;
    }

};