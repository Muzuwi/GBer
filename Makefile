OUTPUT=GBer.exe
CFLAGS= -Wall -Wno-format -std=c++17 -g
LIB=-lmingw32 -lsdl2main -lsdl2 -lopengl32
SRC=imgui/*.cpp ./File/File.cpp ./Memory/RAM.cpp ./Debug/Debug.cpp ./CPU/CPU.cpp ./Math/Math.cpp ./Config/Config.cpp Main.cpp
ADDFLAGS=-fmax-errors=1

default: main

old: $(SRC)
	g++ $(CFLAGS) $(ADDFLAGS) $(SRC) -o $(OUTPUT) $(LIB)

clean:
	$(RM) $(OUTPUT)   

rebuild: clean gber

preproc:
	g++ -Wall -std=c++17 -static -E $(SRC) > preprocessor_output.txt

# .PHONY: imgui
#imgui: ./build/imgui*.o
#	@echo Building imgui
#	cd build/ && g++ $(CFLAGS) ../imgui/*.cpp -lsdl2main -lsdl2 -lopengl32 -c

.PHONY: debug
debug:
	@echo [1 of 7] Building Debug
	g++ $(CFLAGS) Debug/Debug.cpp -c -o build/gber-debug.o

ram: debug
	@echo [2 of 7] Building RAM
	g++ $(CFLAGS) Memory/RAM.cpp -c -o build/gber-ram.o

cpu: math ram debug config
	@echo [3 of 7] Building CPU
	g++ $(CFLAGS) CPU/CPU.cpp -c -o build/gber-cpu.o
	@echo [3.5 of 7] Building PPU
	g++ $(CFLAGS) CPU/PPU.cpp -c -o build/gber-ppu.o

main: debug cpu
	@echo [4 of 7] Building File
	g++ $(CFLAGS) File/File.cpp -c -o build/gber-file.o
	@echo [5 of 7] Building Config
	g++ $(CFLAGS) Config/Config.cpp -c -o build/gber-config.o
	@echo [6 of 7] Building Math
	g++ $(CFLAGS) Math/Math.cpp -c -o build/gber-math.o
	@echo [7 of 7] Building main and linking
	g++ $(CFLAGS) Main.cpp -o GBer.exe build/imgui*.o build/gber-config.o build/gber-debug.o build/gber-math.o build/gber-ram.o build/gber-ppu.o build/gber-cpu.o build/gber-file.o $(LIB)