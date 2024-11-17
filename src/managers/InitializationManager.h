#pragma once

#include <vector>
#include <memory>
#include <iostream>

class IInitializable {
public:
    // Pure virtual function to initialize the module, must be implemented by derived classes.
    virtual bool Init() = 0;
    virtual ~IInitializable() = default; // Default virtual destructor.
};

class InitializationManager {
public:
    // Registers a module that implements the IInitializable interface.
    void Register(std::shared_ptr<IInitializable> initModule) {
        modules.emplace_back(initModule);
    }

    // Initializes all registered modules. Returns false if any module fails to initialize.
    bool InitAll() {
    for (auto& module : modules) {
        if (!module->Init()) {
            std::cerr << "Failed to initialize a module." << std::endl;
            return false;
        }
    }
    return true;
}

    // Shutdown all modules
    void Shutdown() {
        // Implement shutdown logic if needed
    }

private:
    // Vector containing all the modules that need to be initialized.
    std::vector<std::shared_ptr<IInitializable>> modules;
};