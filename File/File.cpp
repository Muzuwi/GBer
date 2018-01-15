#include <iostream>
#include <string>
#include "File.hpp"
#include "../Math/Math.hpp"
namespace File{
		unsigned int fileSize = 0;

        bool fileExists(std::string path){
            boost::filesystem::path p(path);
            return (boost::filesystem::exists(p) & boost::filesystem::is_regular_file(p)) ? true : false;
        }

        std::vector<unsigned char> loadFromFile(std::string path){
        	std::ifstream file;
            std::vector<char> temp;
            file.open(path, std::ios::binary);
            file.seekg(0, file.end);
            fileSize = file.tellg();
            file.seekg(0, file.beg);

            temp.resize(fileSize);
            file.read(&temp[0], fileSize);
            file.close();
            //  std::cout << fileSize << " ";
            //  std::cout << Math::decHex(temp[0x90]) << " ";
            //  rom.resize(fileSize);
            std::vector<unsigned char> rom(temp.begin(), temp.end());
            //  std::cout << Math::decHex(rom[0x90]) << "\n";
            return rom;
        }
}  // namespace File
