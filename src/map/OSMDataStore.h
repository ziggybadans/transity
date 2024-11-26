#pragma once

#include <vector>
#include <shared_mutex>
#include <memory>
#include "SpatialIndex.h"

/* Forward declarations */
class OSMNode;
class OSMWay;
class OSMRelation;

/**
 * @brief Thread-safe storage for OSM map data with spatial indexing
 */
class OSMDataStore {
public:
    OSMDataStore() = default;
    ~OSMDataStore() = default;
    
    void AddNode(OSMNode&& node);
    void AddWay(OSMWay&& way);
    void AddRelation(OSMRelation&& relation);
    
    const std::vector<OSMNode>& GetNodes() const;
    const std::vector<OSMWay>& GetWays() const;
    const std::vector<OSMRelation>& GetRelations() const;

    // New methods for data querying
    std::vector<OSMNode> GetNodesInBounds(double minLat, double maxLat, double minLon, double maxLon) const;
    std::vector<OSMWay> GetWaysInBounds(double minLat, double maxLat, double minLon, double maxLon) const;
    size_t GetTotalElements() const;
    void Clear();

private:
    mutable std::shared_mutex m_mutex;
    std::vector<OSMNode> m_nodes;
    std::vector<OSMWay> m_ways;
    std::vector<OSMRelation> m_relations;
    
    // Add spatial index manager
    SpatialIndexManager m_spatialIndex;
}; 