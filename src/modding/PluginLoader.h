#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include "Plugin.h"

#ifdef _WIN32
#include <windows.h>
#define PLUGIN_EXTENSION ".dll"
#else
#include <dlfcn.h>
#define PLUGIN_EXTENSION ".so"
#endif

class PluginLoader {
public:
    static std::shared_ptr<Plugin> LoadPlugin(const std::filesystem::path& path);
    static void UnloadPlugin(void* handle);

private:
    static void* LoadLibrary(const std::filesystem::path& path);
    static void* GetSymbol(void* handle, const char* symbol);
}; 