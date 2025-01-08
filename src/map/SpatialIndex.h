#pragma once

#include <memory>
#include <vector>
#include <array>
#include <shared_mutex>
#include <mutex>
#include <chrono>
#include <thread>
#include "OSMNode.h"
#include "OSMWay.h"
#include "OSMRelation.h"

/**
 * @brief Represents a bounding box in geographic coordinates
 */
class BoundingBox {
public:
    BoundingBox() = default;
    BoundingBox(double minLat, double maxLat, double minLon, double maxLon)
        : m_minLat(minLat), m_maxLat(maxLat), m_minLon(minLon), m_maxLon(maxLon) {}

    bool Intersects(const BoundingBox& other) const {
        return !(m_maxLat < other.m_minLat || m_minLat > other.m_maxLat ||
                m_maxLon < other.m_minLon || m_minLon > other.m_maxLon);
    }

    bool Contains(double lat, double lon) const {
        return lat >= m_minLat && lat <= m_maxLat &&
               lon >= m_minLon && lon <= m_maxLon;
    }

    double GetMinLat() const { return m_minLat; }
    double GetMaxLat() const { return m_maxLat; }
    double GetMinLon() const { return m_minLon; }
    double GetMaxLon() const { return m_maxLon; }

private:
    double m_minLat{0.0}, m_maxLat{0.0};
    double m_minLon{0.0}, m_maxLon{0.0};
};

/**
 * @brief R-tree node for spatial indexing
 */
template<typename T>
class RTreeNode {
public:
    static constexpr size_t MAX_ENTRIES = 8;
    static constexpr size_t MIN_ENTRIES = MAX_ENTRIES / 2;

    bool IsLeaf() const { return m_isLeaf; }
    const BoundingBox& GetBounds() const { return m_bounds; }

private:
    bool m_isLeaf{true};
    BoundingBox m_bounds;
    std::array<std::unique_ptr<RTreeNode>, MAX_ENTRIES> m_children;
    std::vector<std::pair<BoundingBox, T>> m_entries;
};

/**
 * @brief Thread-safe R-tree implementation for spatial indexing
 */
template<typename T>
class SpatialIndex {
public:
    SpatialIndex() = default;
    ~SpatialIndex() = default;

    /**
     * @brief Insert an element into the spatial index
     * @param element Element to insert
     * @param bounds Bounding box of the element
     */
    void Insert(const T& element, const BoundingBox& bounds) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        InsertInternal(element, bounds);
    }

    /**
     * @brief Bulk load multiple elements into the index
     * @param elements Vector of element-bounds pairs to insert
     */
    void BulkLoad(const std::vector<std::pair<T, BoundingBox>>& elements) {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        BulkLoadInternal(elements);
    }

    /**
     * @brief Query elements within a bounding box
     * @param bounds Bounding box to query
     * @return Vector of elements within the bounds
     */
    std::vector<T> QueryRange(const BoundingBox& bounds) const {
        std::shared_lock<std::shared_mutex> lock(m_mutex);
        return QueryRangeInternal(bounds);
    }

    /**
     * @brief Clear the spatial index
     */
    void Clear() {
        std::unique_lock<std::shared_mutex> lock(m_mutex);
        m_root.reset();
    }

    /**
     * @brief Bulk load multiple elements into the index in parallel
     * @param elements Vector of element-bounds pairs to insert
     * @param threadCount Number of threads to use for parallel processing
     */
    void BulkLoadParallel(const std::vector<std::pair<T, BoundingBox>>& elements,
                         size_t threadCount = std::thread::hardware_concurrency()) {
        auto startTime = std::chrono::high_resolution_clock::now();

        try {
            if (elements.empty()) {
                DEBUG_WARNING("Attempting to bulk load empty element set");
                return;
            }

            if (threadCount == 0) {
                threadCount = 1;
            }

            DEBUG_DEBUG("Starting parallel bulk load of ", elements.size(), 
                       " elements using ", threadCount, " threads");

            // Sort elements by their center latitude (could be improved with Hilbert curve)
            std::vector<std::pair<T, BoundingBox>> sortedElements = elements;
            std::sort(sortedElements.begin(), sortedElements.end(),
                [](const auto& a, const auto& b) {
                    double centerA = (a.second.GetMinLat() + a.second.GetMaxLat()) / 2.0;
                    double centerB = (b.second.GetMinLat() + b.second.GetMaxLat()) / 2.0;
                    return centerA < centerB;
                });

            // Split elements into chunks for parallel processing
            size_t chunkSize = (sortedElements.size() + threadCount - 1) / threadCount;
            std::vector<std::future<std::unique_ptr<RTreeNode<T>>>> futures;
            futures.reserve(threadCount);

            // Process chunks in parallel
            for (size_t i = 0; i < threadCount && i * chunkSize < sortedElements.size(); ++i) {
                size_t startIdx = i * chunkSize;
                size_t endIdx = std::min(startIdx + chunkSize, sortedElements.size());

                futures.push_back(std::async(std::launch::async, [this, &sortedElements, startIdx, endIdx]() {
                    std::vector<std::pair<T, BoundingBox>> chunk(
                        sortedElements.begin() + startIdx,
                        sortedElements.begin() + endIdx
                    );

                    // Create leaf nodes for this chunk
                    size_t elementsPerNode = RTreeNode<T>::MAX_ENTRIES;
                    size_t numLeafNodes = (chunk.size() + elementsPerNode - 1) / elementsPerNode;
                    std::vector<std::unique_ptr<RTreeNode<T>>> leafNodes;
                    leafNodes.reserve(numLeafNodes);

                    size_t elementIndex = 0;
                    while (elementIndex < chunk.size()) {
                        auto leafNode = std::make_unique<RTreeNode<T>>();
                        leafNode->m_isLeaf = true;

                        // Fill node with elements
                        size_t count = std::min(elementsPerNode, chunk.size() - elementIndex);
                        for (size_t j = 0; j < count; ++j) {
                            leafNode->m_entries.push_back(chunk[elementIndex + j]);
                        }

                        // Calculate bounds for the leaf node
                        std::vector<BoundingBox> leafBoxes;
                        for (const auto& entry : leafNode->m_entries) {
                            leafBoxes.push_back(entry.first);
                        }
                        leafNode->m_bounds = CalculateBounds(leafBoxes);

                        leafNodes.push_back(std::move(leafNode));
                        elementIndex += count;
                    }

                    // Build non-leaf levels for this chunk
                    std::vector<std::unique_ptr<RTreeNode<T>>> currentLevel = std::move(leafNodes);
                    
                    while (currentLevel.size() > 1) {
                        std::vector<std::unique_ptr<RTreeNode<T>>> nextLevel;
                        size_t nodesPerParent = RTreeNode<T>::MAX_ENTRIES;
                        size_t numParentNodes = (currentLevel.size() + nodesPerParent - 1) / nodesPerParent;
                        nextLevel.reserve(numParentNodes);

                        size_t nodeIndex = 0;
                        while (nodeIndex < currentLevel.size()) {
                            auto parentNode = std::make_unique<RTreeNode<T>>();
                            parentNode->m_isLeaf = false;

                            size_t count = std::min(nodesPerParent, currentLevel.size() - nodeIndex);
                            for (size_t j = 0; j < count; ++j) {
                                parentNode->m_children[j] = std::move(currentLevel[nodeIndex + j]);
                            }

                            std::vector<BoundingBox> parentBoxes;
                            for (const auto& child : parentNode->m_children) {
                                if (child) {
                                    parentBoxes.push_back(child->GetBounds());
                                }
                            }
                            parentNode->m_bounds = CalculateBounds(parentBoxes);

                            nextLevel.push_back(std::move(parentNode));
                            nodeIndex += count;
                        }

                        currentLevel = std::move(nextLevel);
                    }

                    return std::move(currentLevel[0]);
                }));
            }

            // Collect results from all threads
            std::vector<std::unique_ptr<RTreeNode<T>>> topLevelNodes;
            topLevelNodes.reserve(futures.size());

            for (auto& future : futures) {
                topLevelNodes.push_back(future.get());
            }

            // Create final root node combining all thread results
            auto rootNode = std::make_unique<RTreeNode<T>>();
            rootNode->m_isLeaf = false;

            for (size_t i = 0; i < topLevelNodes.size(); ++i) {
                rootNode->m_children[i] = std::move(topLevelNodes[i]);
            }

            std::vector<BoundingBox> rootBoxes;
            for (const auto& child : rootNode->m_children) {
                if (child) {
                    rootBoxes.push_back(child->GetBounds());
                }
            }
            rootNode->m_bounds = CalculateBounds(rootBoxes);

            // Set the root
            m_root = std::move(rootNode);

            // Calculate and log performance metrics
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

            DEBUG_INFO("Parallel bulk load completed: ",
                      elements.size(), " elements using ",
                      threadCount, " threads in ",
                      duration.count() / 1000.0, "ms");
        }
        catch (const std::exception& e) {
            DEBUG_ERROR("Parallel bulk load failed: ", e.what());
            throw; // Re-throw after logging
        }
    }

private:
    void InsertInternal(const T& element, const BoundingBox& bounds);
    void BulkLoadInternal(const std::vector<std::pair<T, BoundingBox>>& elements);
    std::vector<T> QueryRangeInternal(const BoundingBox& bounds) const;
    
    std::unique_ptr<RTreeNode<T>> m_root;
    mutable std::shared_mutex m_mutex;

    /* Helper Methods */
    static BoundingBox CalculateBounds(const std::vector<BoundingBox>& boxes);
    void SplitNode(RTreeNode<T>* node);
    RTreeNode<T>* ChooseSubtree(const BoundingBox& bounds);

    static constexpr size_t MEMORY_POOL_SIZE = 1024;
    std::vector<std::unique_ptr<RTreeNode<T>>> m_nodePool;
    size_t m_poolIndex{0};

    RTreeNode<T>* AllocateNode() {
        if (m_poolIndex >= m_nodePool.size()) {
            m_nodePool.push_back(std::make_unique<RTreeNode<T>>());
        }
        return m_nodePool[m_poolIndex++].get();
    }

    struct QueryCache {
        BoundingBox bounds;
        std::vector<T> results;
        std::chrono::steady_clock::time_point timestamp;
    };
    static constexpr size_t MAX_CACHE_ENTRIES = 100;
    mutable std::vector<QueryCache> m_queryCache;
    mutable std::mutex m_cacheMutex;
};

/**
 * @brief Manager class for different spatial indices
 */
class SpatialIndexManager {
public:
    SpatialIndexManager() = default;
    ~SpatialIndexManager() = default;

    void AddNode(const OSMNode& node);
    void AddWay(const OSMWay& way);
    void AddRelation(const OSMRelation& relation);
    
    std::vector<OSMNode> GetNodesInBounds(const BoundingBox& bounds) const;
    std::vector<OSMWay> GetWaysInBounds(const BoundingBox& bounds) const;
    std::vector<OSMRelation> GetRelationsInBounds(const BoundingBox& bounds) const;
    
    void Clear();

private:
    BoundingBox CalculateRelationBounds(const OSMRelation& relation) const;
    
    SpatialIndex<OSMNode> m_nodeIndex;
    SpatialIndex<OSMWay> m_wayIndex;
    SpatialIndex<OSMRelation> m_relationIndex;
}; 