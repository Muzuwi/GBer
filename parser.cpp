#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <map>
#include <algorithm>
#include <exception>
std::vector<std::string> Separate(char, std::string);
std::string decHex(int);


int main(){
	std::vector<std::string> lines;
	std::ifstream file;
	file.open("CB3.txt");
	while(!file.eof()){
		std::string temp;
		std::getline(file, temp);
		lines.push_back(temp);
	}
	file.close();
	std::cout << "Lines: " << lines.size() << "\n";

	std::ofstream out;
	out.open("optemplate_CB.txt");
	for(size_t i = 0; i < lines.size(); i++){
		int lower = i / 16;
		int higher = i - 16*lower;
		std::vector<std::string> blocks = Separate('|', lines[i]);

		if(blocks.size() < 2){ 
			std::cout << "\nMalformed entry " << i << "\n";
		}else if(blocks[0] == "DUMMY"){
			std::cout << "Ignoring\n";
			continue;
		}

		int flagMaskSet = 0x0, power = 1;
		for(int j = 3; j >= 0; j--){
			//std::cout << blocks[2].at(j-1);
			if(blocks[2].at(j) == '0'){
				std::cout << decHex(i) << "): " << flagMaskSet << " + " << power << " -> " << flagMaskSet+power << "(Iter " << j << ")\n";
				flagMaskSet += power;
			}
			power *= 2;
		}
		//std::cout << "\n";

		int flagMaskUnset = 0x0;
		power = 1;
		for(int j = 3; j >= 0; j--){
			//std::cout << blocks[2].at(j-1);
			if(blocks[2].at(j) == '1'){
				flagMaskUnset += power;
			}
			power *= 2;
		}
		//std::cout << "\n";

		if(blocks[1].size() > 2){
			std::vector<std::string> cycle = Separate('/', blocks[1]);
			out << "y(0x" << decHex(flagMaskSet) << ", 0x" << decHex(flagMaskUnset) << ", " << "0x" << decHex(lower) << decHex(higher) << "," << blocks[0] << "," << cycle[1] << "," << "                ) \\ \n";
		}else{
			out << "y(0x" << decHex(flagMaskSet) << ", 0x" << decHex(flagMaskUnset) << ", " << "0x" << decHex(lower) << decHex(higher) << "," << blocks[0] << "," << blocks[1] << "," << "                ) \\ \n";
		}
	}
	out.close();
}

std::vector<std::string> Separate(char mode, std::string input){
	unsigned int lastPos = 0, i = 0, currentElement = 0;
	std::string empty;
	std::vector<std::string> output(1);
	for(i = 0; i < input.length(); i++){
		if(input.at(i) == mode){
			output.push_back(empty);
			output.at(currentElement) = input.substr(lastPos, i - lastPos);
			currentElement++;
			lastPos = i + 1;
		}
		if(i == input.length() - 1){
			output.push_back(empty);
			output.at(currentElement) = input.substr(lastPos, i - lastPos + 1);
		}
	}
	return output;
}

std::string decHex(int num){
    std::map<int, char> Hex;
    Hex[0]  = '0';
    Hex[1]  = '1';
    Hex[2]  = '2';
    Hex[3]  = '3';
    Hex[4]  = '4';
    Hex[5]  = '5';
    Hex[6]  = '6';
    Hex[7]  = '7';
    Hex[8]  = '8';
    Hex[9]  = '9';
    Hex[10] = 'A';
    Hex[11] = 'B';
    Hex[12] = 'C';
    Hex[13] = 'D';
    Hex[14] = 'E';
    Hex[15] = 'F';
    if(num == 0){
        return "0";
    }
    std::string text = "";
    int b = num, current = 0, a = 0;
    do{
        a = b % 16;
        text += Hex[a];
        b /= 16;
        current++;
    }while(b != 0);
    /*while(text.size() % 2 != 0){
       text.append(1, '0');
    }*/
    std::reverse(text.begin(), text.end());
    return (text.size() <= 0) ? "?" : text;
}
