#include "utils.h"

int intRandMaxMin(const int v_min, const int v_max){
    return rand() %  std::max(v_max - v_min, 1) + v_min;
}

float floatRandMaxMin(const float v_min, const float v_max){
    return std::fmod (rand(), std::max(v_max - v_min, float(1))) + v_min;
}

void removePathAndExtension(const std::string &full_string, std::string &name){
    name = full_string;
    std::string tmp_str = full_string;
	std::string slash = "/";
    std::string dot = ".";
    std::size_t current, previous = 0;

    //remove path /path/to/
	current = tmp_str.find(slash);
    if (current != std::string::npos) {
        while (current != std::string::npos) {
            name = tmp_str.substr(previous, current - previous);
            previous = current + 1;
            current = tmp_str.find(slash, previous);
        }
        name = tmp_str.substr(previous, current - previous);
    }
    // remove extension
    current = name.find(dot);
    previous = 0;
    if (current != std::string::npos) 
        name = name.substr(previous, current);
    
    // std::cout<<"full string: "<<full_string<<" name: "<<name<<std::endl;
}

