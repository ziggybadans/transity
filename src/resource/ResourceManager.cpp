#include "ResourceManager.h"
#include "../Debug.h"
#include <filesystem>

ResourceManager::ResourceManager(std::unique_ptr<ThreadManager>& threadManager)
    : m_threadManager(threadManager)
{}

bool ResourceManager::LoadResources() {
    /*
    bool success = true;
    std::condition_variable cv;
    std::mutex mutex;
    size_t pendingTasks = 0;
    
    // Load textures with high priority
    m_threadManager->EnqueueTask("LoadTextures", ThreadPriority::High, [this, &success, &cv, &mutex, &pendingTasks]() {
        try {
            const std::filesystem::path texturesPath = "assets/textures";
            for (const auto& entry : std::filesystem::directory_iterator(texturesPath)) {
                if (entry.path().extension() == ".png" || entry.path().extension() == ".jpg") {
                    std::string key = entry.path().stem().string();
                    if (!LoadResource<sf::Texture>(key, entry.path().string())) {
                        DEBUG_ERROR("Failed to load texture: ", entry.path().string());
                        success = false;
                    }
                }
            }
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error loading textures: ", e.what());
            success = false;
        }
        
        {
            std::lock_guard<std::mutex> lock(mutex);
            pendingTasks--;
        }
        cv.notify_one();
    });
    pendingTasks++;

    // Load fonts with normal priority
    m_threadManager->EnqueueTask("LoadFonts", ThreadPriority::Normal, [this, &success, &cv, &mutex, &pendingTasks]() {
        try {
            const std::filesystem::path fontsPath = "assets/fonts";
            for (const auto& entry : std::filesystem::directory_iterator(fontsPath)) {
                if (entry.path().extension() == ".ttf") {
                    std::string key = entry.path().stem().string();
                    if (!LoadResource<sf::Font>(key, entry.path().string())) {
                        DEBUG_ERROR("Failed to load font: ", entry.path().string());
                        success = false;
                    }
                }
            }
        } catch (const std::exception& e) {
            DEBUG_ERROR("Error loading fonts: ", e.what());
            success = false;
        }
        
        {
            std::lock_guard<std::mutex> lock(mutex);
            pendingTasks--;
        }
        cv.notify_one();
    });
    pendingTasks++;

    // Wait for all loading tasks to complete
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait(lock, [&pendingTasks] { return pendingTasks == 0; });
    */
    return true;
} 