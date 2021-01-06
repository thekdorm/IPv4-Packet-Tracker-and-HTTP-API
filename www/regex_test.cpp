#include <iostream>
#include <regex>

int main(){
    std::string url = "/api?ip=192.168.53.1";
    std::regex re("/api(\\?ip=([\\w,\\.]+))?");
    std::smatch m;

    std::regex_match(url, m, re);

    std::cout << m[0] << "; " << m[1] << "; " << m[2] <<std::endl;
}