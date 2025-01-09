#pragma once

#include <vector>

class Line;

struct Segment {
    int startPointIndex;
    int endPointIndex;
    std::vector<Line*> overlappingLines;

    // Constructor
    Segment(int start, int end) : startPointIndex(start), endPointIndex(end) {}
};
