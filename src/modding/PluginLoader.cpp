#include "PluginLoader.h"
#include <stdexcept>
#include <iostream>

std::shared_ptr<Plugin> PluginLoader::LoadPlugin(const std::filesystem::path& path) {
    if (path.extension() != PLUGIN_EXTENSION) {
        throw std::runtime_error("Invalid plugin extension");
    }

    void* handle = LoadLibrary(path);
    if (!handle) {
        throw std::runtime_error("Failed to load plugin library");
    }

    // Get plugin creation function
    using CreatePluginFunc = Plugin* (*)();
    auto createPlugin = reinterpret_cast<CreatePluginFunc>(GetSymbol(handle, "CreatePlugin"));
    
    if (!createPlugin) {
        UnloadPlugin(handle);
        throw std::runtime_error("Failed to get plugin creation function");
    }

    // Create plugin instance
    Plugin* plugin = createPlugin();
    if (!plugin) {
        UnloadPlugin(handle);
        throw std::runtime_error("Failed to create plugin instance");
    }

    return std::shared_ptr<Plugin>(plugin, [handle](Plugin* p) {
        delete p;
        UnloadPlugin(handle);
    });
}

void* PluginLoader::LoadLibrary(const std::filesystem::path& path) {
#ifdef _WIN32
    return LoadLibraryA(path.string().c_str());
#else
    return dlopen(path.c_str(), RTLD_LAZY);
#endif
}

void* PluginLoader::GetSymbol(void* handle, const char* symbol) {
#ifdef _WIN32
    return GetProcAddress((HMODULE)handle, symbol);
#else
    return dlsym(handle, symbol);
#endif
}

void PluginLoader::UnloadPlugin(void* handle) {
#ifdef _WIN32
    FreeLibrary((HMODULE)handle);
#else
    dlclose(handle);
#endif
} 