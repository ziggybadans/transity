#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <memory>

/**
 * @brief Represents a way (line or area) in OpenStreetMap data
 */
class OSMWay {
public:
    OSMWay() = default;
    
    explicit OSMWay(int64_t id)
        : m_id(id)
    {}
    
    ~OSMWay() = default;

    /* Getters */
    int64_t GetId() const { return m_id; }
    const std::vector<int64_t>& GetNodeRefs() const { return m_nodeRefs; }
    const std::unordered_map<std::string, std::string>& GetTags() const { return m_tags; }
    
    bool IsClosed() const {
        return m_nodeRefs.size() >= 3 && m_nodeRefs.front() == m_nodeRefs.back();
    }

    /* Setters */
    void SetId(int64_t id) { m_id = id; }
    
    void AddNodeRef(int64_t nodeId) {
        m_nodeRefs.push_back(nodeId);
        m_boundsCalculated = false; // Invalidate bounds
    }
    
    void AddTag(const std::string& key, const std::string& value) {
        m_tags[key] = value;
    }

    /* Utility */
    bool HasTag(const std::string& key) const {
        return m_tags.find(key) != m_tags.end();
    }

    std::string GetTag(const std::string& key) const {
        auto it = m_tags.find(key);
        return it != m_tags.end() ? it->second : std::string();
    }

    void ClearTags() {
        m_tags.clear();
    }

    void ClearNodeRefs() {
        m_nodeRefs.clear();
        m_boundsCalculated = false;
    }

    /* Geometry */
    bool IsArea() const {
        if (!IsClosed()) return false;
        
        // Check for area tags
        static const std::vector<std::string> areaTags = {
            "area", "building", "landuse", "leisure", "natural", "amenity"
        };
        
        for (const auto& tag : areaTags) {
            if (HasTag(tag)) return true;
        }
        
        return false;
    }

    void CalculateBounds(double& minLat, double& maxLat, double& minLon, double& maxLon) const {
        if (!m_boundsCalculated) {
            // Note: Actual bounds calculation requires node coordinates
            // This will be handled by the OSMDataStore which has access to nodes
            m_minLat = minLat;
            m_maxLat = maxLat;
            m_minLon = minLon;
            m_maxLon = maxLon;
            m_boundsCalculated = true;
        }
        
        minLat = m_minLat;
        maxLat = m_maxLat;
        minLon = m_minLon;
        maxLon = m_maxLon;
    }

private:
    int64_t m_id{0};
    std::vector<int64_t> m_nodeRefs;
    std::unordered_map<std::string, std::string> m_tags;
    mutable bool m_boundsCalculated{false};
    mutable double m_minLat{0.0}, m_maxLat{0.0}, m_minLon{0.0}, m_maxLon{0.0};
}; 