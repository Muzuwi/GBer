#include <iostream>
#include <string>
#include <deque>
#include <SDL.h>
#include <SDL_opengl.h>
#include <Screen.hpp>
#include <PPU.hpp>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl.h"
#include "../imgui/imgui_impl_opengl2.h"
#include "../imgui/imgui_memory_editor.h"
#include "Debug.hpp"
#include "../Math/Math.hpp"
#include "../Memory/RAM.hpp"
#include "../CPU/CPU.hpp"
#include "../Config/Config.hpp"

namespace Debug{
	
	SDL_Window* sdlWindow;
	SDL_GLContext gl_context;
	bool closed = false;
	MemoryEditor memEdit;

	int offset = 0, timer = 0;
	std::string command = "";
	std::string menuText = "";
	bool enterMode = false;
	std::deque<std::string> recentTraces;
	std::vector<char16_t> breakpoints;
	std::vector<unsigned char> bpOp;
	std::vector<MemoryBreakpoint> memoryBreakpoints;
	std::ofstream file;
	bool read , write, prefixCB;

    bool InitGraphicsSubsystems(){
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
        sdlWindow = SDL_CreateWindow("GBer Debugger", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

        if(sdlWindow == nullptr){
            std::cout << "SDL failed to start\n";
            std::cout << SDL_GetError();
            return false;
        }

        SDL_DisplayMode curr;
        SDL_GetCurrentDisplayMode(0, &curr);
        gl_context = SDL_GL_CreateContext(sdlWindow);
        SDL_GL_SetSwapInterval(1); // Enable vsync

        //  Dear Imgui bind
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplSDL2_InitForOpenGL(sdlWindow, gl_context);
        ImGui_ImplOpenGL2_Init();
    
        ImGui::StyleColorsDark();
        return true;
    }
    
    void TerminateGraphics(){
        ImGui_ImplOpenGL2_Shutdown();
        ImGui_ImplSDL2_Shutdown();
        ImGui::DestroyContext();

        SDL_GL_DeleteContext(gl_context);
        SDL_DestroyWindow(sdlWindow);
        SDL_Quit();
    }

	void DebugWindowHandler(){	    
		if(!InitGraphicsSubsystems()){
    	    return;
    	}
	
    	ImGuiIO& io = ImGui::GetIO(); (void)io;
    	ImVec4 clearColor = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    	bool showCpu = true, checkZ, checkN, checkH, checkC, showFlow = true, showBreakpoints = true;
    	while(!closed){
    	    SDL_Event event;
    	    while(SDL_PollEvent(&event)){
    	        ImGui_ImplSDL2_ProcessEvent(&event);
    	        switch(event.type){
    	            case SDL_QUIT: closed = true; break;
    	            default: break;
    	        }
    	    }
			checkZ = (CPU::Registers.F >> 7);
			checkN =  ((CPU::Registers.F & 0x40) >> 6);
			checkH = ((CPU::Registers.F & 0x20) >> 5);
			checkC = ((CPU::Registers.F & 0x10) >> 4);

    	    //  Start dear imgui frame
    	    ImGui_ImplOpenGL2_NewFrame();
    	    ImGui_ImplSDL2_NewFrame(sdlWindow);
    	    ImGui::NewFrame();

    	    //  Window config goes here
    	    //ImGui::ShowDemoWindow(&showDemo);
			memEdit.DrawWindow("GB RAM", &RAM::RAM[0], RAM::RAM.size());

			//  Show CPU info window
			ImGui::Begin("CPU", &showCpu, 0);
			std::string temp;

			temp = "AF: " + Math::decHex(CPU::readAF());
			ImGui::Text(temp.c_str());
			ImGui::SameLine();
			temp = "BC: " + Math::decHex(CPU::readBC());
			ImGui::Text(temp.c_str());

			temp = "DE: " + Math::decHex(CPU::readDE());
			ImGui::Text(temp.c_str());
			ImGui::SameLine();
			temp = "HL: " + Math::decHex(CPU::readHL());
			ImGui::Text(temp.c_str());

			temp = "SP: " + Math::decHex(CPU::Registers.SP);
			ImGui::Text(temp.c_str());
			ImGui::SameLine();
			temp = "PC: " + Math::decHex(CPU::Registers.PC);
			ImGui::Text(temp.c_str());

			ImGui::Checkbox("Z", &checkZ);
			ImGui::SameLine();
			ImGui::Checkbox("N", &checkN);
			ImGui::SameLine();
			ImGui::Checkbox("H", &checkH);
			ImGui::SameLine();
			ImGui::Checkbox("C", &checkC);
			ImGui::End();

			GLuint test;
			glGenTextures(1, &test);

			//  Flow control
			ImGui::Begin("Flow Control", &showFlow);
			if(ImGui::ArrowButton("Continue Execution", ImGuiDir_Right)){
				CPU::continueExec = true;
				CPU::step = true;
			}
			ImGui::SameLine();
			ImGui::Text("Continue");
			if(ImGui::ArrowButton("Trace", ImGuiDir_Down)){
				CPU::continueExec = false;
				CPU::step = true;
			}
			ImGui::SameLine();
			ImGui::Text("Trace Into");
			if(ImGui::Button("Reload")){
				CPU::reload = true;
			}
			if(timer > 0){
				ImGui::Text(menuText.c_str());
			}
			if(CPU::cpuHalt){
				ImGui::Text("HALT");
			}

			ImGui::End();

			//  Breakpoints
			ImGui::Begin("Breakpoints", &showBreakpoints);
			static char buffer[6] = "";
			ImGui::InputText("Address", buffer, IM_ARRAYSIZE(buffer));
			ImGui::Checkbox("R##read", &read);
			ImGui::SameLine();
			ImGui::Checkbox("W##write", &write);
			ImGui::SameLine();
			ImGui::Checkbox("Prefix CB", &prefixCB);

			ImGui::SameLine();
			if(ImGui::Button("Add")){
				if(std::string("").compare(buffer)){
					int casted = strtol(buffer, NULL, 16);
					if(!(casted <= 0 || casted > 0xFFFF)){
						breakpoints.push_back(casted);
					} else if (casted == 0 && (std::string("0").compare(buffer) || std::string("0x0").compare(buffer) )){
						breakpoints.push_back(casted);
					}
				}
			}

			ImGui::SameLine();
			if(ImGui::Button("Add brOP")){
				if(std::string("").compare(buffer)){
					int casted = strtol(buffer, NULL, 16);
					if(!(casted <= 0 || casted > 0xFF)){
						bpOp.push_back(casted);
					}
				}
			}

			ImGui::SameLine();
			if(ImGui::Button("Add memBr")){
				if(std::string("").compare(buffer) && (read || write)){
					int casted = strtol(buffer, NULL, 16);
					if(!(casted <= 0 || casted > 0xFFFF)){
						MemoryBreakpoint br;
						br.addr = casted;
						br.r = read;
						br.w = write;
						memoryBreakpoints.push_back(br);
					}
				}
			}

			//   Breakpoints
			ImGui::BeginChild("breakpointlist");
			ImGui::Columns(3);

			//  Address breakpoints
			if(ImGui::Button("Rmv. All##bk")){
				breakpoints.clear();
			}
			int id = 0, buttonID = 0;
			for(char16_t a : breakpoints){
				ImGui::PushID(buttonID);
				ImGui::BulletText(Math::decHex(a).c_str());
				ImGui::SameLine();
				std::string tag = "x##bp." + Math::decHex(a);
				if(ImGui::SmallButton(tag.c_str())){
					breakpoints.erase(breakpoints.begin()+id);
				}
				++id;
				ImGui::PopID();
				++buttonID;
			}

			//  OP Breakpoints
			ImGui::NextColumn();
			if(ImGui::Button("Rmv. All##bop")){
				bpOp.clear();
			}
			int id2 = 0;
			for(unsigned char a : bpOp){
				ImGui::PushID(buttonID);
				ImGui::BulletText(Math::decHex(a, 2).c_str());
				ImGui::SameLine();
				std::string tag = "x##bOP." + Math::decHex(a);
				if(ImGui::SmallButton(tag.c_str())){
					bpOp.erase(bpOp.begin()+id2);
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
				std::string label = Math::decHex(br.addr) + " [";
				if(br.r) label += "R";
				if(br.w) label += "W";
				label += "]";
				ImGui::BulletText(label.c_str());
				ImGui::SameLine();
				std::string tag = "x##bpM." + Math::decHex(br.addr);
				if(ImGui::SmallButton(tag.c_str())){
					memoryBreakpoints.erase(memoryBreakpoints.begin()+id3);
				}
				++id3;
				ImGui::PopID();
				++buttonID;
			}

			ImGui::EndChild();
			ImGui::End();

			//  Flags
			std::string tempstring;
			ImGui::Begin("Flags");
			tempstring = "DIV: " + Math::decHex(RAM::RAM[0xFF04], 2);
			ImGui::Text(tempstring.c_str());
			tempstring = "TIMA: " + Math::decHex(RAM::RAM[0xFF05], 2);
			ImGui::Text(tempstring.c_str());
			ImGui::Text(("T-Freq: " + std::to_string(CPU::timerFreq)).c_str());
			ImGui::End();


			bool saveStack = false;
			ImGui::Begin("Stack");
			for(int i = 0xFFFE; i >= CPU::Registers.SP; i -= 2){
				std::string label = Math::decHex(i-1) + ": " + Math::decHex(RAM::RAM[i]*0x100 + RAM::RAM[i-1]);
				ImGui::Text(label.c_str());
			}
			if(ImGui::Button("Save")){
				saveStack = true;
			}
			ImGui::SetScrollHere(1.0f);

			if(saveStack){
				file.open("stackdump.bin", std::ios::binary);
				int siz = 0xFFFF - CPU::Registers.SP;
				file.write((char*)&RAM::RAM[CPU::Registers.SP], siz * sizeof(unsigned char));
			}
			file.close();

			saveStack = false;
			ImGui::End();

			ImGui::Begin("Trace");
			for(std::string a : recentTraces){
				ImGui::Text(a.c_str());
			}
			ImGui::End();


			ImGui::Begin("Screen / PPU");
			{
				ImGui::SliderInt("Scale Screen", &Screen::scale, 1, 5);
				ImGui::NewLine();
				ImGui::Text(("LY: " + Math::decHex(RAM::RAM[0xFF44], 2)).c_str());
				ImGui::Text(("cyclesSinceModeSwitch: " + std::to_string(PPU::cyclesSinceModeSwitch)).c_str());
				ImGui::Text(("Fifo size: " + std::to_string(Screen::fifo.size())).c_str());

				std::string mode;
				switch(PPU::currentPpuMode){
					case PPU::OAM:
						mode = "OAM";
						break;
					case PPU::VBLANK:
						mode = "VBlank";
						break;
					case PPU::HBLANK:
						mode = "HBlank";
						break;
					case PPU::PIXTX:
						mode = "Pixel Transfer";
						break;
				}
				ImGui::Text(("PPU Mode: " + mode).c_str());
				ImGui::Text(("Vblank Lines: " + std::to_string(PPU::vblankLines)).c_str());
				ImGui::Text(("FIFO Fetches: " + std::to_string(PPU::pixelTransferFinishedFetches)).c_str());


			}
			ImGui::End();

    	    //  Render frame
    	    ImGui::Render();
    	    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    	    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    	    glClear(GL_COLOR_BUFFER_BIT);
    	    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    	    SDL_GL_SwapWindow(sdlWindow);

			timer = (timer > 0) ? timer - 1 : 0;
			if(timer == 0){
				menuText = "";
			}
    	}
    	TerminateGraphics();
	}

	void emuLog(std::string message, LEVEL lvl){
		std::cout << (lvl == 10 ? "[@] " : \
					   lvl == 5 ? "[!] " : "[i] " ) << message << "\n";
 	}

 	void emuLog(std::string message){
 		std::cout << "[i] " << message << "\n";
 	} 
	 
	/*bool vectorGetterChar16(void* data, int n, const char** out_text){
		std::vector<char16_t>* v = (std::vector<char16_t>*)data;
		if(n < 0 || n >= v->size()) return false;
		*out_text = Math::decHex(v->at(n)).c_str();
		return true;
	}*/

}