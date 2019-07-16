#include "Headers/Utils.hpp"

namespace Utils{
    /*
        Returns hex form of given integer
    */
    std::string decHex(int num, unsigned int size){
        std::stringstream stream;
        stream << std::hex << num << std::dec;
        std::string out = stream.str();
        while(out.size() < size){
            out.insert(0,"0");
        }
        return out;
    }

    //  Because Mingw sucks and <filesystem> is broken
    bool exists(std::string path){
        std::ifstream infile(path);
        return infile.good();
    }

    std::vector<uint8_t> loadFromFile(std::string path){
        std::ifstream file;
        std::vector<char> temp;
        file.open(path, std::ios::binary);
        file.seekg(0, file.end);
        size_t fileSize = file.tellg();
        file.seekg(0, file.beg);

        temp.resize(fileSize);
        file.read(&temp[0], fileSize);
        file.close();
        //  std::cout << fileSize << " ";
        //  std::cout << Math::decHex(temp[0x90]) << " ";
        //  rom.resize(fileSize);
        std::vector<uint8_t> rom(temp.begin(), temp.end());
        //  std::cout << Math::decHex(rom[0x90]) << "\n";
        return rom;
    }



}