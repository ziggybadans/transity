// src/Utilities.h
#pragma once

class Utilities {
public:
    // Wrap a floating-point coordinate within a maximum value
    static float wrapFloat(float coord, float max);

    // Wrap an integer coordinate within a maximum value
    static int wrapCoordinate(int coord, int max);

    // Calculate the number of steps needed to iterate from start to end with wrapping
    static int calculateSteps(int start, int end, int max);
};