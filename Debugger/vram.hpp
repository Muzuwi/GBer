#pragma once
#include "imgui/imgui.h"
#include <GL/gl3w.h>
#include "Headers/RAM.hpp"

class DebugVRAM{
    const std::string windowName = "VRAM Viewer";

    //  Flip tile display by X/Y axis
    bool flipTilesX, flipTilesY;

    //  Framebuffers for tile map and background map textures
    unsigned int tileMapBuffer[16*8*3*64];
    unsigned int backgroundMapBuffer[0x20*0x20*64];
    //unsigned int* tileMapBuffer = new unsigned int[16*8*3*64];
    //unsigned int* backgroundMapBuffer = new unsigned int[0x20*0x20*64];


    //  GL texture IDs for tile map and background map display
    GLuint textureID, backgroundMapTextureID;

    //  Tile/Background map texture width and height
    unsigned int tileMapWidth, tileMapHeight,
                 backgroundMapWidth, backgroundMapHeight;

    //  Colors for the viewers
    SDL_Color colors[4];

public:
    bool open = false;

    void cleanup(){
        //delete[] tileMapBuffer;
        //delete[] backgroundMapBuffer;
    }

    void init(){
        tileMapWidth = 16*8;
        tileMapHeight = 8*3*8;

        backgroundMapWidth = 0x20*8;
        backgroundMapHeight = 0x20*8;

        for(unsigned int i = 0; i < tileMapHeight*tileMapWidth; i++) tileMapBuffer[i] = 0xFFFFFFFF;
        for(unsigned int i = 0; i < backgroundMapWidth*backgroundMapHeight; i++) backgroundMapBuffer[i] = 0xFFFFFFFF;

        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);
        //  THIS IS IMPORTANT, OTHERWISE THE TEXTURE DRAWN IS BLACK
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tileMapWidth, tileMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tileMapBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenTextures(1, &backgroundMapTextureID);
        glBindTexture(GL_TEXTURE_2D, backgroundMapTextureID);
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri ( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0x20*8, 0x20*8, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backgroundMapBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);

        colors[0].r = colors[0].g = colors[0].b = 255;
        colors[1].r = colors[1].g = colors[1].b = 170;
        colors[2].r = colors[2].g = colors[2].b = 85;
        colors[3].r = colors[3].g = colors[3].b = 0;

    }

    inline void updateTileData(ImGuiIO* io, RAM* ram, PPU* ppu){
        ImGui::Checkbox("XFlip", &flipTilesX);
        ImGui::SameLine();
        ImGui::Checkbox("YFlip", &flipTilesY);

        //  Draw nice tiles
        for(int i = 0; i < (0x97F0 - 0x8000)/0x10 + 1; i++){
            unsigned int posX = i%0x10,
                    posY = i/0x10;
            for(int line = 0; line < 8; line++){
                //  Get tile data
                uint8_t* tileBuffer = new uint8_t[8];
                if(i <= 0x7F){
                    ppu->debugGetRawTileDataWrapper(i, line, tileBuffer, 8, 0, 8, flipTilesX, flipTilesY,0x8000);
                } else {
                    ppu->debugGetRawTileDataWrapper(i-0x80, line, tileBuffer, 8, 0, 8, flipTilesX, flipTilesY,0x9000);
                }

                for(int pxl = 0; pxl < 8; pxl++){
                    unsigned int arrayPosY = posY*8 + line,
                            arrayPosX = posX*8 + pxl;

                    uint32_t pixel = 0;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].r) << 24;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].g) << 16;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].b) << 8;
                    pixel |= 0xFF;

                    assert(arrayPosY*tileMapWidth + arrayPosX <= tileMapHeight*tileMapWidth);
                    tileMapBuffer[arrayPosY*tileMapWidth + arrayPosX] = pixel;

                }

                delete[] tileBuffer;
            }
        }
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tileMapWidth, tileMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, tileMapBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);

        const unsigned int scale = 2;

        //  Draw tile preview on mouse over
        ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::Image((void*)textureID, ImVec2(tileMapWidth*scale, tileMapHeight*scale));
        if(ImGui::IsItemHovered()){
            ImGui::BeginTooltip();
            float size = 64.0f;
            float tileX = (int)(io->MousePos.x - pos.x) / (8*scale), tileY = (int)(io->MousePos.y - pos.y) / (8*scale);
            ImVec2 topLeft = ImVec2(tileX*8,tileY*8),
                    uv0 = ImVec2(topLeft.x / tileMapWidth, topLeft.y / tileMapHeight),
                    uv1 = ImVec2( (topLeft.x + 8) / tileMapWidth, (topLeft.y + 8) / tileMapHeight);
            ImGui::Image((void*)textureID, ImVec2(size, size), uv0, uv1);

            ImGui::Text( ("Tile ID: " + Utils::decHex((((int)tileY % 0x10) << 4)|(int)tileX)).c_str());
            ImGui::EndTooltip();

        }
    }

    inline void updateMap(RAM* ram, PPU* ppu){
        //unsigned int dataAddr;

        for(int i = 0; i < (ppu->getLCDC()->bgTileMap.higher - ppu->getLCDC()->bgTileMap.lower); i++){
            uint8_t tileID = ram->peek(ppu->getLCDC()->bgTileMap.lower + i);
            unsigned int posX = i%0x20,
                    posY = i/0x20;

            for(int j = 0; j < 8; j++){
                //  Get raw tile data at line j
                uint8_t* tileBuffer = new uint8_t[8];
                ppu->debugGetRawTileDataWrapper(tileID, j, tileBuffer, 8, 0, 8, false, false, 0);

                //  Add pixels to the buffer
                for(int pxl = 0; pxl < 8; pxl++){
                    unsigned int arrayPosY = posY*8 + j,
                            arrayPosX = posX*8 + pxl;

                    uint32_t pixel = 0;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].r) << 24;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].g) << 16;
                    pixel |= ((uint32_t)colors[tileBuffer[pxl]].b) << 8;
                    pixel |= 0xFF;

                    backgroundMapBuffer[arrayPosY*backgroundMapWidth + arrayPosX] = pixel;
                }
                delete[] tileBuffer;
            }


        }
        //  Draw viewport
        for(int i = ram->peek(SCX); i < 160+ram->peek(SCX); i++){
            if(i == ram->peek(SCX) || i == 160+ram->peek(SCX)-1){
                for(int j = ram->peek(SCY); j < ram->peek(SCY) + 144; j++){
                    unsigned int y = j % (0x20*8);
                    backgroundMapBuffer[y*backgroundMapWidth + i] = 0xFF0000FF;
                }
            }
            unsigned int x = i % (0x20*8);
            //  Draw top
            backgroundMapBuffer[ram->peek(SCY)*backgroundMapWidth + x] = 0xFF0000FF;
            //  Draw bottom
            backgroundMapBuffer[((ram->peek(SCY)+144)%backgroundMapHeight)*backgroundMapWidth + x] = 0xFF0000FF;
        }
        glBindTexture(GL_TEXTURE_2D, backgroundMapTextureID);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, backgroundMapWidth, backgroundMapHeight, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, backgroundMapBuffer);
        glBindTexture(GL_TEXTURE_2D, 0);

        ImGui::Image((void*)backgroundMapTextureID, ImVec2(0x20*8, 0x20*8));
    }

    void updateWindow(ImGuiIO* io, PPU* ppu, RAM* ram){
        if(!open) return;
        ImGui::Begin("VRAM", &this->open);
        if(ImGui::BeginTabBar("##vramtabs", ImGuiTabBarFlags_None)){
            if(ImGui::BeginTabItem("Tile Data")){
                updateTileData(io, ram, ppu);
                ImGui::EndTabItem();
            }
            if(ImGui::BeginTabItem("Background Map")){
               updateMap(ram, ppu);
               ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }


};