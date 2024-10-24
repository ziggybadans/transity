#pragma once
#include <vector>
#include <memory>

class IInitializable {
public:
    virtual bool Init() = 0;
    virtual ~IInitializable() = default;
};

class InitializationManager {
public:
    void Register(std::shared_ptr<IInitializable> initModule) {
        modules.emplace_back(initModule);
    }

    bool InitAll() {
        for (auto& module : modules) {
            if (!module->Init()) {
                return false;
            }
        }
        return true;
    }

private:
    std::vector<std::shared_ptr<IInitializable>> modules;
};
