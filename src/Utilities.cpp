// src/Utilities.cpp
#include "Utilities.h"
#include <cmath>

// Wrap a floating-point coordinate within a maximum value
float Utilities::wrapFloat(float coord, float max) {
    float result = std::fmod(coord, max);
    if (result < 0)
        result += max;
    return result;
}

// Wrap an integer coordinate within a maximum value
int Utilities::wrapCoordinate(int coord, int max) {
    return (coord % max + max) % max;
}

// Calculate the number of steps needed to iterate from start to end with wrapping
int Utilities::calculateSteps(int start, int end, int max) {
    if (end >= start) {
        return end - start + 1;
    }
    else {
        return (max - start) + (end + 1);
    }
}
