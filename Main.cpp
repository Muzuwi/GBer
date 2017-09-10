#include <iostream>
#include "File/File.hpp"

int main(){

	std::vector<char> test = File::loadFromFile("testing");
	std::cout << "Test file size " << test.size() << "\n";

	return 0;
}