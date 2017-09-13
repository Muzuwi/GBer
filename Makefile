OUTPUT=GBer.exe
CFLAGS= -Wall -O3 -g -std=c++17
LIB=-lboost_filesystem -lboost_system
SRC=./File/File.cpp ./Memory/RAM.cpp ./CPU/CPU.cpp ./Math/Math.cpp Main.cpp

default: gber

gber: $(SRC)
	g++ $(CFLAGS) $(SRC) $(LIB) -o $(OUTPUT)

clean:
	$(RM) $(OUTPUT)   
