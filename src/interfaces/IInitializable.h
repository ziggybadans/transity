#pragma once

class IInitializable {
public:
    // Pure virtual function to initialize the module, must be implemented by derived classes.
    virtual bool Init() = 0;
    virtual ~IInitializable() = default; // Default virtual destructor.
}; 