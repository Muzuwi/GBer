OUTPUT=GBer.exe
CFLAGS= -Wall -g -std=c++17 -static
LIB=-lsfml-graphics -lsfml-system -lsfml-window
SRC=./File/File.cpp ./Memory/RAM.cpp ./Debug/Debug.cpp ./CPU/CPU.cpp ./Math/Math.cpp ./Config/Config.cpp Main.cpp
ADDFLAGS=-fmax-errors=1

default: gber

gber: $(SRC)
	g++ $(CFLAGS) $(ADDFLAGS) $(SRC) $(LIB) -o $(OUTPUT)

clean:
	$(RM) $(OUTPUT)   

rebuild: clean gber

debug:
	g++ -Wall -std=c++17 -static -E $(SRC) > preprocessor_output.txt
