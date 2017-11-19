OUTPUT=GBer.exe
CFLAGS= -Wall -O3 -g -std=c++17 -static
LIB=-lboost_filesystem -lboost_system
SRC=./File/File.cpp ./Memory/RAM.cpp ./CPU/CPU.cpp ./Math/Math.cpp ./Config/Config.cpp Main.cpp

default: gber

gber: $(SRC)
	g++ $(CFLAGS) $(SRC) $(LIB) -o $(OUTPUT)

clean:
	$(RM) $(OUTPUT)   
