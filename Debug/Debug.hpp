#pragma once
#include <string>
#include <vector>
#include <deque>
#include <SDL.h>
#include <SDL_opengl.h>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl.h"
#include "../imgui/imgui_impl_opengl2.h"

namespace Debug{
	struct MemoryBreakpoint{
		char16_t addr;
		bool r = false, w = false;
		unsigned char val = 0;
	};

	extern std::deque<std::string> recentTraces;
	extern int timer;
	extern std::string command;
	extern std::string menuText;
	extern std::vector<char16_t> breakpoints;
	extern std::vector<unsigned char> bpOp;
	extern std::vector<MemoryBreakpoint> memoryBreakpoints;

	enum LEVEL{
		ERR=10,
		WARN=5,
		INFO=1
	};

	void emuLog(std::string, LEVEL);
	void emuLog(std::string);
	void processDebuggerCommand();
	bool InitGraphicsSubsystems();
	void TerminateGraphics();
	void DebugWindowHandler();
}