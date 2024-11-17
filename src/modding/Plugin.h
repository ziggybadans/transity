#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <any>

enum class PluginType {
    Asset,      // For texture/sound/model mods
    Gameplay,   // For gameplay modifications
    Interface   // For UI modifications
};

class Plugin {
public:
    Plugin(const std::string& name, const std::string& author, const std::string& version, PluginType type)
        : m_name(name)
        , m_author(author)
        , m_version(version)
        , m_type(type)
        , m_isEnabled(false)
    {}

    virtual ~Plugin() = default;

    /* Core Plugin Methods */
    virtual bool Load() = 0;
    virtual void Unload() = 0;
    virtual void Update(float dt) = 0;

    /* Getters */
    const std::string& GetName() const { return m_name; }
    const std::string& GetAuthor() const { return m_author; }
    const std::string& GetVersion() const { return m_version; }
    PluginType GetType() const { return m_type; }
    bool IsEnabled() const { return m_isEnabled; }

protected:
    std::string m_name;
    std::string m_author;
    std::string m_version;
    PluginType m_type;
    bool m_isEnabled;
}; 