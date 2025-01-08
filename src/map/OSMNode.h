#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * @brief Represents a node (point) in OpenStreetMap data
 */
class OSMNode {
public:
    OSMNode() = default;
    
    OSMNode(int64_t id, double lat, double lon)
        : m_id(id)
        , m_latitude(lat)
        , m_longitude(lon)
    {}
    
    ~OSMNode() = default;

    /* Getters */
    int64_t GetId() const { return m_id; }
    double GetLatitude() const { return m_latitude; }
    double GetLongitude() const { return m_longitude; }
    const std::unordered_map<std::string, std::string>& GetTags() const { return m_tags; }

    /* Setters */
    void SetId(int64_t id) { m_id = id; }
    void SetLatitude(double lat) { m_latitude = lat; }
    void SetLongitude(double lon) { m_longitude = lon; }
    
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

private:
    int64_t m_id{0};
    double m_latitude{0.0};
    double m_longitude{0.0};
    std::unordered_map<std::string, std::string> m_tags;
}; 