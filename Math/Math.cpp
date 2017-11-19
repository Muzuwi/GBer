#include <map>
#include <algorithm>
#include <iostream>

namespace Math{

    /*
        Returns hex form of given integer
    */
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
             return "00";
         }
         std::string text = "";
         int b = num, current = 0, a = 0;
         do{
             a = b % 16;
             text += Hex[a];
             b /= 16;
             current++;
         }while(b != 0);
         while(text.size() % 2 != 0){
            text.append(1, '0');
         } 
         std::reverse(text.begin(), text.end());
         
         return (text.size() <= 0) ? "?" : text; 
    }
/*
    template<class T>
    T pow(T a, unsigned int b){
        T ret = a;
        unsigned int count = 1;
        do{
            ret *= a;
            count++;
        }while(count != b);
        return ret;
    }*/
}
