cmake_minimum_required(VERSION 3.12)
project(GBer)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)
set(CMAKE_CXX_STANDARD 17)

include_directories(Config)
include_directories(CPU)
include_directories(Debug)
include_directories(File)
include_directories(imgui)
include_directories(Math)
include_directories(Memory)

add_executable(GBer
        Config/Config.cpp
        Config/Config.hpp
        CPU/CPU.cpp
        CPU/CPU.hpp
        CPU/PPU.cpp
        CPU/PPU.hpp
        Debug/Debug.cpp
        Debug/Debug.hpp
        File/File.cpp
        File/File.hpp
        imgui/gl3w.c
        imgui/gl3w.h
        imgui/glcorearb.h
        imgui/imconfig.h
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_opengl2.cpp
        imgui/imgui_impl_opengl2.h
        imgui/imgui_impl_sdl.cpp
        imgui/imgui_impl_sdl.h
        imgui/imgui_internal.h
        imgui/imgui_memory_editor.h
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        Math/Math.cpp
        Math/Math.hpp
        Memory/RAM.cpp
        Memory/RAM.hpp
        Main.cpp
        Makefile CPU/Screen.cpp CPU/Screen.hpp)

target_link_libraries(GBer -lmingw32)
target_link_libraries(GBer -lsdl2main)
target_link_libraries(GBer -lsdl2)
target_link_libraries(GBer -lopengl32)