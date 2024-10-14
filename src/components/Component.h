// src/components/Component.h
#pragma once

struct Position {
    float x;
    float y;
};

struct Velocity {
    float vx;
    float vy;
};

// New Components
struct Terrain {
    enum class Type { Land, Water } type;
};

struct Height {
    float value;
};
