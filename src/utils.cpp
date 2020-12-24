#include "utils.h"

int intRandMaxMin(const int v_min, const int v_max){
    return rand() %  std::max(v_max - v_min, 1) + v_min;
}

float floatRandMaxMin(const float v_min, const float v_max){
    return std::fmod (rand(), std::max(v_max - v_min, float(1))) + v_min;
}


