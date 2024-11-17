#pragma once

#include <vector>
#include <memory>
#include <iostream>
#include "../Debug.h"
#include "../interfaces/IInitializable.h"

class InitializationManager {
public:
    // Registers a module that implements the IInitializable interface.
    void Register(std::shared_ptr<IInitializable> initModule) {
        modules.emplace_back(initModule);
    }

    // Initializes all registered modules. Returns false if any module fails to initialize.
    bool InitAll() {
        DEBUG_INFO("Starting initialization of ", modules.size(), " modules");
        for (auto& module : modules) {
            if (!module->Init()) {
                DEBUG_ERROR("Module initialization failed");
                return false;
            }
            DEBUG_DEBUG("Module initialized successfully");
        }
        DEBUG_INFO("All modules initialized successfully");
        return true;
    }

    // Shutdown all modules
    void Shutdown() {
        // Clear all registered modules
        modules.clear();
    }

private:
    // Vector containing all the modules that need to be initialized.
    std::vector<std::shared_ptr<IInitializable>> modules;
};