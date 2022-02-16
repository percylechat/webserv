#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <istream>

int main(){
    std::string file1 = "norminet.jpg";
    std::basic_ifstream<char> fin(file1.c_str(), std::ios::binary);
	std::ostringstream oss;
	oss << fin.rdbuf();
	std::string data(oss.str());
    std::string file2 = "lol.jpg";
    std::basic_ofstream<char> lol(file2.c_str(), std::ios::binary);
    // std::istringstream tp;
    lol << data;
    // lol << tp.rdbuf();
    lol.close();
}