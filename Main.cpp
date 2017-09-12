#include <iostream>
#include "File/File.hpp"
#include "Memory/RAM.hpp"

int main(int argc, char* argv[]){
	if(argc < 2){
		return -1;
	}
	std::vector<char> rom = File::loadFromFile(argv[1]);
	std::cout << "File size " << rom.size() << "\n";
	std::cout << "Trying to insert into mem...";
	RAM::insert(rom, 0x0150, 0x3EB0);
	RAM::dump("notmem.txt");

	return 0;
}