# Transport Management Game - Modding Guide
## Table of Contents
1. Introduction
2. Plugin Types
3. Creating Asset Mods
4. Creating Gameplay Mods
5. Plugin Structure
5. API Reference

## 1. Introduction
The Transport Management Game features a robust modding system that allows for both asset modifications and gameplay extensions. All mods are loaded in a sandboxed environment for security.

### Security Considerations
Plugins are executed in a sandboxed environment with the following restrictions:
- Limited API access to approved game functions
- Restricted filesystem access to mod directories only
- Memory usage limits
- Thread creation limits
- File size restrictions

Attempting to access system functions, execute arbitrary code, or exceed resource limits will cause the plugin to be rejected or terminated.

## 2. Plugin Types
There are three main types of plugins:
- Asset Plugins: For adding or modifying textures, sounds, models
- Gameplay Plugins: For modifying game mechanics and adding features
- Interface Plugins: For modifying or adding UI elements

## 3. Creating Asset Mods
### Basic Asset Plugin Structure
```cpp
#include "PluginAPI.h"

class CustomAssetPlugin : public Plugin {
public:
    CustomAssetPlugin() 
        : Plugin("MyAssets", "AuthorName", "1.0", PluginType::Asset) 
    {}

    bool Load() override {
        // Load your custom assets here
        return true;
    }

    void Unload() override {
        // Cleanup when mod is disabled
    }

    void Update(float dt) override {
        // Usually empty for asset plugins
    }
};

extern "C" {
    PLUGIN_API Plugin* CreatePlugin() {
        return new CustomAssetPlugin();
    }
}
```

### Adding Custom Textures
```cpp
bool Load() override {
    auto resourceManager = GetResourceManager();
    return resourceManager->LoadResource<sf::Texture>(
        "custom_train", 
        "assets/textures/train.png"
    ) != nullptr;
}
```

## 4. Creating Gameplay Mods
### Basic Gameplay Plugin Structure
```cpp
class CustomGameplayPlugin : public Plugin {
public:
    CustomGameplayPlugin() 
        : Plugin("CustomGameplay", "AuthorName", "1.0", PluginType::Gameplay) 
    {}

    bool Load() override {
        // Register new game mechanics
        RegisterEventHandlers();
        return true;
    }

    void Update(float dt) override {
        // Update custom game logic
    }

    void Unload() override {
        // Cleanup custom systems
    }

private:
    void RegisterEventHandlers() {
        auto eventManager = GetEventManager();
        eventManager->Subscribe(EventType::KeyPressed, 
            [this](const sf::Event& event) {
                // Handle custom input
            }
        );
    }
};
```

## 5. Plugin Structure
Your mod folder should follow this structure:
```
mods/
  ├── my_mod/
  │   ├── manifest.json
  │   ├── plugin.dll
  │   ├── assets/
  │   │   ├── textures/
  │   │   ├── sounds/
  │   │   └── models/
  │   └── scripts/
```

### manifest.json Example
```json
{
    "name": "Custom Transport Pack",
    "version": "1.0.0",
    "author": "Your Name",
    "description": "Adds new transport types",
    "type": "gameplay",
    "dependencies": [],
    "load_order": 1,
    "api_version": "1.0"
}
```

## 6. API Reference
### Available Managers
- ResourceManager: For asset loading and management
- EventManager: For handling game events
- InputManager: For custom input handling
- UIManager: For UI modifications

### Key Base Classes
- Plugin: Base class for all plugins
- IInitializable: Interface for initialization
- OSMFeature: For map feature modifications