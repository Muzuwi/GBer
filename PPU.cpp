#include "Headers/PPU.hpp"
#include "Headers/Emulator.hpp"
#include "Headers/RAM.hpp"
#include "Headers/Display.hpp"

//  Bind an emulator object to this module
void PPU::bind(Emulator* newEmulator){
    emulator = newEmulator;
}

void PPU::setModePPU(PPU_MODE mode){
    currentModePPU = mode;
}

void PPU::clearCycles(){
    ppuCycles = 0;
}

/*
 *  Updates the LCDC configuration in the class
 */
void PPU::updateVariables(){
    auto memory = emulator->getMemory();
    uint8_t lcdcVal = memory->peek(LCDC),
            statVal = memory->peek(STAT);

    if((lcdcVal & 0x8)){
        lcdc.bgTileMap.lower = 0x9C00;
        lcdc.bgTileMap.higher = 0x9FFF;
    } else {
        lcdc.bgTileMap.lower = 0x9800;
        lcdc.bgTileMap.higher = 0x9BFF;
    }

    if((lcdcVal & 0x10)){
        lcdc.bgWindowTileData.lower = 0x8000;
        lcdc.bgWindowTileData.higher = 0x8FFF;
        lcdc.tileAccessModeSigned = false;
    } else {
        lcdc.bgWindowTileData.lower = 0x8800;
        lcdc.bgWindowTileData.higher = 0x97FF;
        lcdc.tileAccessModeSigned = true;
    }

    if((lcdcVal & 0x40)){
        lcdc.windowTileMap.lower = 0x9C00;
        lcdc.windowTileMap.higher = 0x9FFF;
    } else {
        lcdc.windowTileMap.lower = 0x9800;
        lcdc.windowTileMap.higher = 0x9BFF;
    }

    if((lcdcVal & 0x80)){
        lcdc.lcdEnable = true;
    } else {
        lcdc.lcdEnable = false;
    }

    if((lcdcVal & 0x20)){
        lcdc.windowEnable = true;
    } else {
        lcdc.windowEnable = false;
    }

    if((lcdcVal & 0x1)){
        lcdc.bgWindowDisplayPriority = true;
    } else {
        lcdc.bgWindowDisplayPriority = false;
    }

    if((lcdcVal & 0x2)){
        lcdc.objEnable = true;
    } else {
        lcdc.objEnable = false;
    }

    if(lcdcVal & 0x4){
        lcdc.objHeight = 16;
    } else {
        lcdc.objHeight = 8;
    }

    //  Set STAT variables
    if(statVal & 0x8){
        intHBL = true;
    } else {
        intHBL = false;
    }

    if(statVal & 0x10){
        intVBL = true;
    } else {
        intVBL = false;
    }

    if(statVal & 0x20){
        intOAM = true;
    } else {
        intOAM = false;
    }

    if(statVal & 0x40){
        intLYC = true;
    } else {
        intLYC = false;
    }

}

/*
 *  Get color number for a sprite after applying the current pallete
 */
inline uint8_t PPU::getSpritePalleteData(uint8_t num, bool pallete1){
    if(pallete1){
        return (emulator->getMemory()->peek(OBP1) & (3 << 2*num)) >> (2*num);
    } else {
        return (emulator->getMemory()->peek(OBP0) & (3 << 2*num)) >> (2*num);
    }
}

/*
 *  Get color number for a tile after applying the current pallete
 */
inline uint8_t PPU::getTilePalleteData(uint8_t num){
    return (emulator->getMemory()->peek(BGP) & (3 << 2*num)) >> (2*num);
}

/*
 *  Fetches the current line of pixels from the background layer
 *  TODO: Refactor this from a 'Blocks-per-line' approach to 'Pixels-per-line'
 */
inline void PPU::fetchCurrentLineBackground(uint8_t *pixels, uint8_t *raw){
    //  Base tile map address
    //  Calculate scrollX / scrollY
    int addr = PPU::lcdc.bgTileMap.lower;

#define BLOCK_COUNT 20

    uint8_t scx = emulator->getMemory()->peek(SCX),
            scy = emulator->getMemory()->peek(SCY),
            ly  = emulator->getMemory()->peek(LY);

    //  When SCX is not a multiple of 8, we need to get 21 tiles instead of 20
    unsigned int add = 0;
    if(scx % 8 != 0) add = 1;

    //  Which block are we drawing?
    uint16_t mapStartBlockX = scx / 8,
            mapStartBlockY = ((scy+ly)/8)%0x20;
    unsigned int x = 0;
    for(unsigned int i = 0; i < BLOCK_COUNT + add; i++){
        //  Get tile id and the address its' tile data is stored in
        uint8_t tileID = emulator->getMemory()->peek(addr + mapStartBlockY*0x20 + mapStartBlockX);
        //  Wrap around addresses on the same line
        mapStartBlockX = (mapStartBlockX + 1) % 0x20;

        //  Get the k - Lines and l - pixels that make up a sprite and draw them
        //  Get 2 bytes making up the tile from ram
        unsigned int spriteLine = (ly + scy) % 8;

        //  Take SCX/SCY into consideration when calculating
        unsigned int startAtXOffset = (scx % 8);

        unsigned int offset = 0, bound = 0;
        //  Starting X of the line
        if(i == 0 && startAtXOffset != 0){
            offset = startAtXOffset;
        } else {
            offset = 0;
        }

        //  Ending X of the line
        if(i == BLOCK_COUNT + add - 1 && startAtXOffset != 0){
            bound = startAtXOffset;
        } else {
            bound = 8;
        }
        assert(offset < bound);

        //  Create a buffer to store tile line pixels
        const size_t bufferSize = bound - offset;
        uint8_t lineBuffer[32];
        assert(bufferSize < 32);

        //  Get line pixels
        getRawTileData(tileID,spriteLine, lineBuffer, bufferSize, offset, bound);
        //  Fill the main line buffer with the pixels
        for(unsigned int pixel = 0; pixel < bufferSize; pixel++){
            //  Check if buffer is overflowing
            assert(x < 160);
            pixels[x] = getTilePalleteData(lineBuffer[pixel]);
            raw[x] = lineBuffer[pixel];
            x++;
        }
    }
}

/*
*  Fetches the current line of pixels from the window layer
*/
inline void PPU::fetchCurrentLineWindow(uint8_t *pixels, uint8_t *raw){
    uint8_t ly  = emulator->getMemory()->peek(LY),
            wy  = emulator->getMemory()->peek(WY),
            wx  = emulator->getMemory()->peek(WX);

    uint8_t drawingLine = ly - wy;
    //unsigned int x = wx - 7, tileNumber = (wx - 7)%8;
    unsigned int x = wx - 7, tileNumber = 0;
    while(x < 160){
        uint8_t tileID = emulator->getMemory()->peek(lcdc.windowTileMap.lower + ((drawingLine)/8)*0x20 + tileNumber);
        uint8_t lineBuffer[8];
        getRawTileData(tileID, drawingLine%8, lineBuffer, 8, 0, 8);
        for(int pixel = 0; pixel < 8; pixel++) {
            pixels[x] = getTilePalleteData(lineBuffer[pixel]);
            raw[x] = lineBuffer[pixel];
            x++;
            if(x == 160) {
                return;
            }
        }
        tileNumber++;
    }
}

/*
*  Fetches the current line of pixels from the sprite layer
*/
inline void PPU::fetchCurrentLineSprite(uint8_t *pixels, const uint8_t *raw){
    auto memory = emulator->getMemory();
    uint8_t /*scx = emulator->getMemory()->peek(SCX),
            scy = emulator->getMemory()->peek(SCY),*/
            ly  = emulator->getMemory()->peek(LY);


    //  Look for sprites that can be drawn to the screen
    OAM_Entry spriteTable[SPRITES_PER_LY];
    unsigned int count = 0;
    //bool temp = false;
    for(unsigned int i = OAM_START; i < OAM_END; i += 4){
        uint8_t yPos = memory->peek(i),
                xPos = memory->peek(i+1),
                patternNumber = memory->peek(i+2),
                flags = memory->peek(i+3);

        if(yPos == 0 || xPos == 0 || yPos >= 160) {
            //if(temp) emulator->getDebugger()->emuLog("OAM entry @ " + Utils::decHex(i) + " failed: out-of-bounds position (" + std::to_string(xPos) + ", " + std::to_string(yPos) + ")");
            continue;
        }
        if(lcdc.objHeight == 8 && yPos < 9) {
            //if(temp) emulator->getDebugger()->emuLog("OAM entry @ " + Utils::decHex(i) + " failed: out-of-bounds position (height 8)");
            continue;
        }
        if(!(ly >= yPos - 16 && ly < yPos - 16 + lcdc.objHeight)){
            //if(temp) emulator->getDebugger()->emuLog("OAM entry @ " + Utils::decHex(i) + " failed: not in current line");
            continue;
        }

        /*if(i == 0xFE78 && patternNumber == 0x80) temp = true;

        if(temp){
            emulator->getDebugger()->emuLog("OAM entry @ " + Utils::decHex(i) + ", pattern " + Utils::decHex(patternNumber));
        }*/

        //  Set properties of the OAM entry
        OAM_Entry temp;
        temp.patternNumber = patternNumber;
        temp.posX = xPos;
        temp.posY = yPos;
        temp.priority = (flags & 0x80);
        temp.xFlip = flags & 0x40;
        temp.yFlip = flags & 0x20;
        temp.obj1Palette = flags & 0x10;
        spriteTable[count] = temp;

        //  The gameboy can only display 10 sprites per scanline
        if(++count == SPRITES_PER_LY) {
            //std::cout << "10 sprite per scanline limit reached\n";
            break;
        }

    }

    //  Drawing the current scanline of OAM sprites
    for(size_t i = 0; i < count; i++){
        OAM_Entry entry = spriteTable[i];
        bool drawingSecond = (ly >= entry.posY - 8 && ly <= entry.posY && lcdc.objHeight == 16);
        if(entry.xFlip) drawingSecond = !drawingSecond;
        //  If the sprite is outside visible range, skip
        //  You can hide sprites like this, but it's better to hide them by setting posY instead,
        //  As changing posX affects the 10 sprites chosen for displaying
        if(entry.posX >= 0xA8) continue;

        //  In 8x16 mode, LSB of pattern is treated as 0
        /*if(lcdc.objHeight == 16) {
            if(!drawingSecond){
                entry.patternNumber &= 0xFE;
            } else {
                entry.patternNumber |= 1;
            }
        }*/

        //  Screen x coordinate of top-left corner of the sprite
        unsigned int x = 0;

        //  Which pixel in sprite line is the first
        unsigned int tileStartX = 0;
        if(entry.posX < 8){
            tileStartX = 8 - entry.posX;
            x = 0;
        } else {
            tileStartX = 0;
            x = entry.posX - 8;
        }

        //  End line pixel X
        unsigned int tileEndX = 8;
        if(entry.posX > 0x9F && entry.posX < 0xA9) {
            tileEndX = 8 - (entry.posX - 0x9F);
        } else {
            tileEndX = 8;
        }

        unsigned int spriteLine = std::abs(ly - (entry.posY - 16));

        const size_t bufferSize = tileEndX - tileStartX;
        uint8_t lineBuffer[32];
        assert(bufferSize < 32);

        //  Get tile data
        if(drawingSecond){
            getRawTileData(entry.patternNumber+1,spriteLine%8,lineBuffer,bufferSize,tileStartX,tileEndX,entry.xFlip,entry.yFlip,0x8000);
        } else {
            getRawTileData(entry.patternNumber,spriteLine%8,lineBuffer,bufferSize,tileStartX,tileEndX,entry.xFlip,entry.yFlip,0x8000);
        }

        //  Insert into tile buffer
        for(unsigned int pixel = 0; pixel < bufferSize; pixel++){
            uint8_t rawNum = lineBuffer[pixel],
                    num = getSpritePalleteData(rawNum, entry.obj1Palette),
                    bgWindowNum = raw[x];
            assert(x < 160);

            if (!entry.priority && rawNum != 0) {
                pixels[x] = num;
            } else if (bgWindowNum == 0 && rawNum != 0) {
                pixels[x] = num;
            } /*else {
                pixels[x] = 4 + rawNum;
            }*/

            x++;
        }
    }
}

/*
*  Draws the current scanline
*  Handles drawing of Background, Sprite, and Window layers
*/
inline void PPU::drawCurrentLine(){
    uint8_t wx = emulator->getMemory()->peek(WX),
            wy = emulator->getMemory()->peek(WY),
            ly = emulator->getMemory()->peek(LY);

    //  Line buffers
    //  Stores pixel data after applying pallete swapping
    uint8_t linePixels[GAMEBOY_SCREEN_WIDTH] {0};

    //  Framebuffer and create SDL texture to store the framebuffer
    uint32_t lineFramebufferData[GAMEBOY_SCREEN_WIDTH] {0};

    //  Raw pixel number buffer, for sprite priority to work correctly
    uint8_t rawBgWindow[GAMEBOY_SCREEN_WIDTH] {0};

    //  If window/background is enabled
    if(lcdc.bgWindowDisplayPriority) {
        //  Get pixels from current background scanline
        fetchCurrentLineBackground(linePixels, rawBgWindow);
    }

    //  If window/background is enabled
    if(lcdc.bgWindowDisplayPriority) {
        //  Get pixels from current window line (if enabled)
        bool windowVisible = (wx >= 0 && wx <= 166) && (wy >= 0 && wy <= 143) && lcdc.windowEnable;
        bool currentLineIsWindow = (ly >= wy);
        if(windowVisible && currentLineIsWindow){
            fetchCurrentLineWindow(linePixels, rawBgWindow);
        }
    }

    //  Get pixels from current sprite scanline
    if(lcdc.objEnable){
        fetchCurrentLineSprite(linePixels, rawBgWindow);
    }


    //  Create framebuffer
    for(unsigned int i = 0; i < GAMEBOY_SCREEN_WIDTH; i++){
        uint8_t num = linePixels[i];
        uint32_t dword = 0;
        if(num == 4){
            dword = 0xFFFF00FF;
        } else if(num == 5) {
            dword = 0xFF0000FF;
        } else if(num == 6) {
            dword = 0x00FF00FF;
        } else if(num == 7) {
            dword = 0x0000FFFF;
        } else {
            dword |= ((uint32_t)emulator->getDisplay()->getColor(num).r) << 24;
            dword |= ((uint32_t)emulator->getDisplay()->getColor(num).g) << 16;
            dword |= ((uint32_t)emulator->getDisplay()->getColor(num).b) << 8;
            dword |= 0xFF;
        }
        lineFramebufferData[i] = dword;
    }

    //  Upload framebuffer to Display module
    emulator->getDisplay()->appendBufferedLine(lineFramebufferData, GAMEBOY_SCREEN_WIDTH, ly);
}

inline void PPU::getRawTileData(uint8_t tileNumber,
                                unsigned int line,
                                uint8_t* array,
                                size_t arraySize,
                                unsigned int pixelOffset,
                                unsigned int bound,
                                bool xflip,
                                bool yflip,
                                uint16_t overrideDataAddress) {
    assert(pixelOffset <= bound);
    assert(line >= 0 && line <= 7);
    assert(pixelOffset >= 0 && pixelOffset <= 8);
    assert(bound >= 0 && bound <= 8);

    uint16_t dataAddr = 0;
    if(overrideDataAddress != 0x0){
        dataAddr = overrideDataAddress + tileNumber*0x10;
    } else if(!lcdc.tileAccessModeSigned){
        dataAddr = lcdc.bgWindowTileData.lower + tileNumber*0x10;
        assert(dataAddr <= lcdc.bgWindowTileData.higher);
    } else {
        //  TODO: Replace magic numbers
        if(tileNumber <= 0x7F){
            dataAddr = 0x9000 + tileNumber*0x10;
        } else {
            dataAddr = 0x8800 + (tileNumber - 0x80)*0x10;
        }
        assert(dataAddr <= lcdc.bgWindowTileData.higher);
    }

    if(xflip){
        line = 7 - line;
    }

    auto memory = emulator->getMemory();
    unsigned int data1 = memory->peek(dataAddr + 2*line),
            data2 = memory->peek(dataAddr + 2*line + 1);

    unsigned int x = 0;
    if(yflip){
        //  Flip order
        for(unsigned int l = bound-1; l >= pixelOffset; l--){
            if(x == arraySize) return;

            //  Get color associated with pixel
            unsigned int mask = static_cast<unsigned int>(1 << (8 - l - 1));
            unsigned int bit1 = (data1 & mask) >> (8-l-1),
                    bit2 = (data2 & mask) >> (8-l-1) << 1,
                    num = bit1|bit2;
            //  Get coords
            assert(!(num < 0 || num > 3));

            array[x++] = num;
        }
    } else {
        for(unsigned int l = pixelOffset; l < bound; l++){
            if(x == arraySize) return;

            //  Get color associated with pixel
            unsigned int mask = (1 << (8 - l - 1));
            unsigned int bit1 = (data1 & mask) >> (8-l-1),
                    bit2 = ((data2 & mask) >> (8-l-1)) << 1,
                    num = bit1|bit2;
            //  Get coords
            assert(!(num < 0 || num > 3));

            array[x++] = num;
        }
    }
}

/*
 *  Updates the PPU and prepares the framebuffer
 */

bool PPU::updateModePPU(int64_t delta){
    ppuCycles += delta;
    //  RAM base pointer
    auto RAM = emulator->getMemory()->getBaseMemoryPointer();

    bool continueUpdate, frameUpdate = false;
    do{
        continueUpdate = false;
        //  OAM/HBlank Mode
        bool oamContinueUpdate;
        do{
            oamContinueUpdate = false;

            //  OAM
            if(currentModePPU == OAM && ppuCycles >= OAM_CYCLE_COUNT){
                drawCurrentLine();
                currentModePPU = HBLANK;
                ppuCycles -= OAM_CYCLE_COUNT;
                oamContinueUpdate = true;
                continueUpdate = true;

                //  Set LCDC bits 0-1 to HBlank
                RAM[STAT] = (RAM[STAT]&0b11111100) | 0b00;

                //      INTERRUPTS

                //  Spawn HBlank interrupt
                if(intHBL && (RAM[IE] & IE_LCDC) && emulator->getCPU()->getRegisters().IME){
                    RAM[IF] |= IF_LCDC;
                    return false;
                } else {
                    //emulator->getDebugger()->emuLog("int: " + std::to_string(intHBL) + ", IE: " + std::to_string(RAM[IE]) + ", IME: " + std::to_string(emulator->getCPU()->getRegisters().IME));
                }
            }

            //  HBlank
            if(currentModePPU == HBLANK && ppuCycles >= HBLANK_CYCLE_COUNT){
                currentModePPU = OAM;
                RAM[LY] += 1;
                ppuCycles -= HBLANK_CYCLE_COUNT;
                oamContinueUpdate = true;
                continueUpdate = true;

                //  Set LCDC bits 0-1 to OAM
                RAM[STAT] = (RAM[STAT] & 0b11111100) | 0b10;

                //  Set LY==LYC coincidence
                if(RAM[LY] == RAM[LYC]){
                    RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b100;
                } else {
                    RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b000;
                }

                //  VBlank Enter
                if(RAM[LY] == 144){
                    currentModePPU = VBLANK;
                    //  Set LCDC bits 0-1 to VBlank
                    RAM[STAT] = (RAM[STAT] & 0b11111100) | 0b01;

                    //  Spawn VBlank interrupt
                    if((RAM[IE] & IE_VBLANK) && emulator->getCPU()->getRegisters().IME){
                        RAM[IF] |= IF_VBLANK;
                        return false;
                    }
                }

                //      INTERRUPTS

                //  Spawn OAM ints
                if(intOAM && (RAM[IE] & IE_LCDC) && emulator->getCPU()->getRegisters().IME){
                    RAM[IF] |= IF_LCDC;
                    return false;
                }

                //  Spawn LYC interrupt
                if(intLYC && RAM[LY] == RAM[LYC] && emulator->getCPU()->getRegisters().IME && (RAM[IE] & IE_LCDC)){
                    RAM[IF] |= IF_LCDC;
                    return false;
                }
            }
        }while(RAM[LY] <= 143 && oamContinueUpdate);


        //  VBlank Mode
        while(RAM[LY] >= 144 && RAM[LY] < 153 && ppuCycles >= VBLANK_CYCLE_COUNT){
            ppuCycles -= VBLANK_CYCLE_COUNT;
            RAM[LY] += 1;

            //  Set LY == LYC coincidence in STAT
            if(RAM[LY] == RAM[LYC]){
                RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b100;
            } else {
                RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b000;
            }

            //      INTERRUPTS

            //  Spawn LCDC -> LY interrupt
            if(intLYC && RAM[LY] == RAM[LYC] && emulator->getCPU()->getRegisters().IME && (RAM[IE] & IE_LCDC)){
                RAM[IF] |= IF_LCDC;
                return false;
            }
        }


        //  Exiting VBlank
        if(RAM[LY] == 153){
            //  TODO: Check proper timing for VBlank to OAM turnout
            if(ppuCycles >= VBLANK_TO_OAM_CYCLE_COUNT + VBLANK_CYCLE_COUNT){
                //  TODO: Check if correct
                RAM[LY] = 0;
                ppuCycles -= VBLANK_TO_OAM_CYCLE_COUNT + VBLANK_CYCLE_COUNT;
                currentModePPU = OAM;

                frameUpdate = true;

                //  Set LY==LYC coincidence
                if(RAM[LY] == RAM[LYC]){
                    RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b100;
                } else {
                    RAM[STAT] = (RAM[STAT] & 0b11111011) | 0b000;
                }

                //  Set LCDC bits 0-1 to OAM
                RAM[STAT] = (RAM[STAT]&0b11111100) | 0b10;

                //      INTERRUPTS

                if(intLYC && RAM[LY] == RAM[LYC] && emulator->getCPU()->getRegisters().IME && (RAM[IE] & IE_LCDC)){
                    RAM[IF] |= IF_LCDC;
                    return true;
                }
                //  Spawn OAM ints
                if(intOAM && (RAM[IE] & IE_LCDC) && emulator->getCPU()->getRegisters().IME){
                    RAM[IF] |= IF_LCDC;
                    return true;
                }

                continueUpdate = true;
            }
        }

    } while(continueUpdate);
    return frameUpdate;

}

void PPU::reload(){
    currentModePPU = OAM;
    emulator->getMemory()->poke(LY, 0);
    ppuCycles = 0;
}

PPU_MODE PPU::getPPUMode(){
    return currentModePPU;
}

LCDController* PPU::getLCDC(){
    return &lcdc;
}

int64_t PPU::getPPUCycles(){
    return ppuCycles;
}

/*
 *  Wrapper for getRawTileData for use in the debugger class
 */
void PPU::debugGetRawTileDataWrapper(uint8_t tileNumber,
                                unsigned int line,
                                uint8_t* array,
                                size_t arraySize,
                                unsigned int pixelOffset,
                                unsigned int bound,
                                bool xflip,
                                bool yflip,
                                uint16_t overrideDataAddress){
    getRawTileData(tileNumber, line, array, arraySize, pixelOffset, bound, xflip,yflip, overrideDataAddress);
}
