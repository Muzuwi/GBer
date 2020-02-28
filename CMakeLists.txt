cmake_minimum_required(VERSION 3.12)
project(GBer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 17)
add_definitions(-O2)
add_definitions(-g)
add_definitions(-Wall)
add_definitions(-pedantic)

include_directories(${CMAKE_SOURCE_DIR}/)

add_library(imgui
        imgui/gl3w.c
        imgui/imgui.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_opengl3.cpp
        imgui/imgui_impl_sdl.cpp
        imgui/imgui_widgets.cpp
        )


add_executable(GBer
        Utils.cpp
        Config.cpp
        MBC/BasicMBC.cpp
        MBC/ROM.cpp
        MBC/MBC1.cpp
        MBC/MBC3.cpp
        MBC/MBC5.cpp
        RAM.cpp
        LR35902.cpp
        Display.cpp
        PPU.cpp
        Debugger.cpp
        Debugger/DebuggerModules.cpp
        APU.cpp
        Emulator.cpp
        Main.cpp
        )

target_link_libraries(GBer imgui)

if(WIN32)
    target_link_libraries(GBer -lmingw32)
    target_link_libraries(GBer -lsdl2main)
    target_link_libraries(GBer -lsdl2)
    target_link_libraries(GBer -lopengl32)
endif(WIN32)

if(UNIX)
    target_link_libraries(GBer -lGL)
    target_link_libraries(GBer -ldl)
    target_link_libraries(GBer -lSDL2main)
    target_link_libraries(GBer -lSDL2)
endif(UNIX)