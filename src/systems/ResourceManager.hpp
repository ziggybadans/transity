// src/systems/ResourceManager.hpp
#pragma once
#include "ResourceManager.h"

template <typename Resource>
Resource& ResourceManager<Resource>::load(const std::string& name, const std::string& filename) {
    auto resource = std::make_unique<Resource>();
    if (!resource->loadFromFile(filename)) {
        throw std::runtime_error("ResourceManager::load - Failed to load " + filename);
    }
    Resource& ref = *resource;
    resources[name] = std::move(resource);
    return ref;
}

template <typename Resource>
Resource& ResourceManager<Resource>::get(const std::string& name) {
    return *resources.at(name);
}
