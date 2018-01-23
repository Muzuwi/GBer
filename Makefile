OUTPUT=GBer.exe
CFLAGS= -Wall -g -std=c++17 -static
LIB=-lboost_filesystem -lboost_system
SRC=./File/File.cpp ./Memory/RAM.cpp ./CPU/CPU.cpp ./Math/Math.cpp ./Config/Config.cpp Main.cpp
ADDFLAGS=-fmax-errors=1

default: gber

gber: $(SRC)
	g++ $(CFLAGS) $(ADDFLAGS) $(SRC) $(LIB) -o $(OUTPUT)

clean:
	$(RM) $(OUTPUT)   

rebuild: clean gber

debug:
	g++ -Wall -std=c++17 -static -E $(SRC) $(LIB) > preprocessor_output.txt
