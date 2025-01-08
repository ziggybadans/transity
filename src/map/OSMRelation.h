#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <unordered_map>

/**
 * @brief Represents a relation (collection of nodes, ways, and other relations) in OpenStreetMap data
 */
class OSMRelation {
public:
    enum class MemberType {
        Node,
        Way,
        Relation
    };

    struct Member {
        MemberType type;
        int64_t ref;
        std::string role;

        Member(MemberType t, int64_t r, const std::string& ro)
            : type(t), ref(r), role(ro) {}
    };

    OSMRelation() = default;
    
    explicit OSMRelation(int64_t id)
        : m_id(id)
    {}
    
    ~OSMRelation() = default;

    /* Getters */
    int64_t GetId() const { return m_id; }
    const std::vector<Member>& GetMembers() const { return m_members; }
    const std::unordered_map<std::string, std::string>& GetTags() const { return m_tags; }

    /* Setters */
    void SetId(int64_t id) { m_id = id; }
    
    void AddMember(MemberType type, int64_t ref, const std::string& role) {
        m_members.emplace_back(type, ref, role);
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

    void ClearMembers() {
        m_members.clear();
    }

    /* Type Checking */
    bool IsMultipolygon() const {
        return HasTag("type") && GetTag("type") == "multipolygon";
    }

    bool IsRoute() const {
        return HasTag("type") && GetTag("type") == "route";
    }

    bool IsBoundary() const {
        return HasTag("type") && GetTag("type") == "boundary";
    }

private:
    int64_t m_id{0};
    std::vector<Member> m_members;
    std::unordered_map<std::string, std::string> m_tags;
}; 