#pragma once

#include <memory>
#include <mutex>
#include <condition_variable>
#include "../world/WorldMap.h"
#include "../utility/ThreadPool.h"

class ResourceManager {
public:
    ResourceManager(std::unique_ptr<ThreadPool>& threadPool);
    ~ResourceManager() = default;

    bool LoadResources();
    std::shared_ptr<WorldMap> GetWorldMap() const { return m_worldMap; }

private:
    std::unique_ptr<ThreadPool>& m_threadPool;
    std::shared_ptr<WorldMap> m_worldMap;
    std::mutex m_worldMapMutex;
}; 