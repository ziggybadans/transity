#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager(std::unique_ptr<ThreadPool>& threadPool)
    : m_threadPool(threadPool)
{}

bool ResourceManager::LoadResources() {
    // Load basic resources like textures and fonts here
    // For now, return true as we don't have any resources to load yet
    return true;
} 