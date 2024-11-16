#include "ResourceManager.h"
#include <iostream>

ResourceManager::ResourceManager(std::unique_ptr<ThreadPool>& threadPool)
    : m_threadPool(threadPool)
{}

bool ResourceManager::LoadResources() {
    std::condition_variable cv;
    bool loaded = false;
    std::mutex cvMutex;

    Task loadWorldMapTask([this, &cv, &loaded]() {
        auto tempWorldMap = std::make_shared<WorldMap>(
            "assets/land_shapes.json",
            "assets/features/cities.geojson",
            "assets/features/towns.geojson",
            "assets/features/suburbs.geojson"
        );
        
        if (tempWorldMap->Init()) {
            {
                std::lock_guard<std::mutex> lock(m_worldMapMutex);
                m_worldMap = tempWorldMap;
                loaded = true;
            }
            cv.notify_one();
        }
        else {
            std::cerr << "Failed to initialize WorldMap." << std::endl;
        }
    });
    
    m_threadPool->enqueueTask(loadWorldMapTask);

    std::unique_lock<std::mutex> lock(cvMutex);
    cv.wait(lock, [&loaded]() { return loaded; });

    return m_worldMap != nullptr;
} 