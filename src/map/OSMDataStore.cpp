#include "OSMDataStore.h"
#include "OSMNode.h"
#include "OSMWay.h"
#include "OSMRelation.h"
#include "SpatialIndex.h"
#include "../Debug.h"
#include <algorithm>
#include <chrono>
#include <stdexcept>

void OSMDataStore::AddNode(OSMNode&& node) {
    // Acquire unique lock for writing
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    // Store node ID for logging
    int64_t nodeId = node.GetId();
    double lat = node.GetLatitude();
    double lon = node.GetLongitude();
    
    // Add to spatial index before moving
    m_spatialIndex.AddNode(node);
    
    // Move node into vector
    m_nodes.push_back(std::move(node));
    
    // Log debug info
    DEBUG_DEBUG("Added node ", nodeId, 
                " at (", lat, ", ", lon, 
                "), total nodes: ", m_nodes.size());
}

void OSMDataStore::AddWay(OSMWay&& way) {
    // Acquire unique lock for writing
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    // Store way ID and node count for logging
    int64_t wayId = way.GetId();
    size_t nodeCount = way.GetNodeRefs().size();
    
    // Add to spatial index before moving
    m_spatialIndex.AddWay(way);
    
    // Move way into vector
    m_ways.push_back(std::move(way));
    
    // Log debug info
    DEBUG_DEBUG("Added way ", wayId,
                " with ", nodeCount, " nodes",
                ", total ways: ", m_ways.size(),
                (m_ways.back().IsArea() ? " (area)" : " (line)"));
}

void OSMDataStore::AddRelation(OSMRelation&& relation) {
    // Acquire unique lock for writing
    std::unique_lock<std::shared_mutex> lock(m_mutex);
    
    // Store relation ID and member count for logging
    int64_t relationId = relation.GetId();
    size_t memberCount = relation.GetMembers().size();
    
    // Add to spatial index before moving
    m_spatialIndex.AddRelation(relation);
    
    // Move relation into vector
    m_relations.push_back(std::move(relation));
    
    // Log debug info with relation type
    std::string relationType;
    if (m_relations.back().IsMultipolygon()) {
        relationType = "multipolygon";
    } else if (m_relations.back().IsRoute()) {
        relationType = "route";
    } else if (m_relations.back().IsBoundary()) {
        relationType = "boundary";
    } else {
        relationType = "other";
    }
    
    DEBUG_DEBUG("Added relation ", relationId,
                " (", relationType, ")",
                " with ", memberCount, " members",
                ", total relations: ", m_relations.size());
}

const std::vector<OSMNode>& OSMDataStore::GetNodes() const {
    // Acquire shared lock for concurrent read access
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_nodes;
}

const std::vector<OSMWay>& OSMDataStore::GetWays() const {
    // Acquire shared lock for concurrent read access
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_ways;
}

const std::vector<OSMRelation>& OSMDataStore::GetRelations() const {
    // Acquire shared lock for concurrent read access
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    return m_relations;
}

std::vector<OSMNode> OSMDataStore::GetNodesInBounds(double minLat, double maxLat, double minLon, double maxLon) const {
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<OSMNode> results;

    try {
        // Validate coordinate bounds
        if (minLat > maxLat || minLon > maxLon) {
            DEBUG_ERROR("Invalid bounds: ",
                       "Lat(", minLat, ", ", maxLat, "), ",
                       "Lon(", minLon, ", ", maxLon, ")");
            throw std::invalid_argument("Invalid coordinate bounds");
        }

        if (minLat < -90.0 || maxLat > 90.0 || minLon < -180.0 || maxLon > 180.0) {
            DEBUG_WARNING("Bounds outside valid range: ",
                        "Lat(", minLat, ", ", maxLat, "), ",
                        "Lon(", minLon, ", ", maxLon, ")");
            // Clamp to valid ranges
            minLat = std::max(-90.0, minLat);
            maxLat = std::min(90.0, maxLat);
            minLon = std::max(-180.0, minLon);
            maxLon = std::min(180.0, maxLon);
        }

        // Create bounding box for query
        BoundingBox queryBounds(minLat, maxLat, minLon, maxLon);

        // Acquire shared lock for reading
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        // Query nodes using spatial index
        results = m_spatialIndex.GetNodesInBounds(queryBounds);

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_DEBUG("Found ", results.size(), " nodes in bounds ",
                   "Lat(", minLat, ", ", maxLat, "), ",
                   "Lon(", minLon, ", ", maxLon, ") ",
                   "in ", duration.count() / 1000.0, "ms");

        // Log additional info if significant nodes were found
        size_t taggedNodes = std::count_if(results.begin(), results.end(),
            [](const OSMNode& node) { return !node.GetTags().empty(); });

        if (taggedNodes > 0) {
            DEBUG_DEBUG(taggedNodes, " of ", results.size(), 
                       " nodes have tags");
        }

        return results;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error querying nodes in bounds: ", e.what());
        throw; // Re-throw after logging
    }
}

std::vector<OSMWay> OSMDataStore::GetWaysInBounds(double minLat, double maxLat, double minLon, double maxLon) const {
    auto startTime = std::chrono::high_resolution_clock::now();
    std::vector<OSMWay> results;

    try {
        // Validate coordinate bounds
        if (minLat > maxLat || minLon > maxLon) {
            DEBUG_ERROR("Invalid bounds: ",
                       "Lat(", minLat, ", ", maxLat, "), ",
                       "Lon(", minLon, ", ", maxLon, ")");
            throw std::invalid_argument("Invalid coordinate bounds");
        }

        if (minLat < -90.0 || maxLat > 90.0 || minLon < -180.0 || maxLon > 180.0) {
            DEBUG_WARNING("Bounds outside valid range: ",
                        "Lat(", minLat, ", ", maxLat, "), ",
                        "Lon(", minLon, ", ", maxLon, ")");
            // Clamp to valid ranges
            minLat = std::max(-90.0, minLat);
            maxLat = std::min(90.0, maxLat);
            minLon = std::max(-180.0, minLon);
            maxLon = std::min(180.0, maxLon);
        }

        // Create bounding box for query
        BoundingBox queryBounds(minLat, maxLat, minLon, maxLon);

        // Acquire shared lock for reading
        std::shared_lock<std::shared_mutex> lock(m_mutex);

        // Query ways using spatial index
        results = m_spatialIndex.GetWaysInBounds(queryBounds);

        // Post-process ways to ensure they actually intersect the bounds
        results.erase(
            std::remove_if(results.begin(), results.end(),
                [this, &queryBounds](const OSMWay& way) {
                    double wayMinLat, wayMaxLat, wayMinLon, wayMaxLon;
                    way.CalculateBounds(wayMinLat, wayMaxLat, wayMinLon, wayMaxLon);
                    BoundingBox wayBounds(wayMinLat, wayMaxLat, wayMinLon, wayMaxLon);
                    return !wayBounds.Intersects(queryBounds);
                }
            ),
            results.end()
        );

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_DEBUG("Found ", results.size(), " ways in bounds ",
                   "Lat(", minLat, ", ", maxLat, "), ",
                   "Lon(", minLon, ", ", maxLon, ") ",
                   "in ", duration.count() / 1000.0, "ms");

        // Log additional info about way types
        size_t areaCount = 0;
        size_t highwayCount = 0;
        size_t buildingCount = 0;
        size_t waterCount = 0;

        for (const auto& way : results) {
            if (way.IsArea()) areaCount++;
            if (way.HasTag("highway")) highwayCount++;
            if (way.HasTag("building")) buildingCount++;
            if (way.HasTag("water") || way.HasTag("waterway")) waterCount++;
        }

        if (!results.empty()) {
            DEBUG_DEBUG("Way types in bounds: ",
                       areaCount, " areas, ",
                       highwayCount, " highways, ",
                       buildingCount, " buildings, ",
                       waterCount, " water features");
        }

        return results;
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error querying ways in bounds: ", e.what());
        throw; // Re-throw after logging
    }
}

size_t OSMDataStore::GetTotalElements() const {
    // Acquire shared lock for reading
    std::shared_lock<std::shared_mutex> lock(m_mutex);
    
    // Sum the sizes of all containers
    size_t total = m_nodes.size() + m_ways.size() + m_relations.size();
    
    DEBUG_DEBUG("Total OSM elements: ", total, 
                " (Nodes: ", m_nodes.size(),
                ", Ways: ", m_ways.size(),
                ", Relations: ", m_relations.size(), ")");
    
    return total;
}

void OSMDataStore::Clear() {
    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Store current sizes for logging
        size_t nodeCount = 0;
        size_t wayCount = 0;
        size_t relationCount = 0;
        
        {
            // Acquire unique lock for writing
            std::unique_lock<std::shared_mutex> lock(m_mutex);
            
            // Store counts before clearing
            nodeCount = m_nodes.size();
            wayCount = m_ways.size();
            relationCount = m_relations.size();
            
            // Clear spatial index
            m_spatialIndex.Clear();
            
            // Clear all vectors
            m_nodes.clear();
            m_ways.clear();
            m_relations.clear();
            
            // Shrink vectors to minimize memory usage
            m_nodes.shrink_to_fit();
            m_ways.shrink_to_fit();
            m_relations.shrink_to_fit();
        }

        // Calculate and log performance metrics
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);

        DEBUG_INFO("Cleared OSMDataStore: ",
                  nodeCount, " nodes, ",
                  wayCount, " ways, ",
                  relationCount, " relations ",
                  "in ", duration.count() / 1000.0, "ms");

        // Log memory usage info
        size_t nodesMemory = m_nodes.capacity() * sizeof(OSMNode);
        size_t waysMemory = m_ways.capacity() * sizeof(OSMWay);
        size_t relationsMemory = m_relations.capacity() * sizeof(OSMRelation);
        
        DEBUG_DEBUG("Memory usage after clear: ",
                   "Nodes: ", nodesMemory / 1024, "KB, ",
                   "Ways: ", waysMemory / 1024, "KB, ",
                   "Relations: ", relationsMemory / 1024, "KB");
    }
    catch (const std::exception& e) {
        DEBUG_ERROR("Error clearing OSMDataStore: ", e.what());
        throw; // Re-throw after logging
    }
} 