#pragma once

#include <string>
#include <unordered_map>
#include <any>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>

enum class SettingType {
    Integer,
    Float,
    Boolean,
    String,
    Vector2u
};

struct SettingDefinition {
    std::string name;
    std::string category;
    SettingType type;
    std::any defaultValue;
    std::function<void(const std::any&)> onChange;
    std::function<bool(const std::any&)> validator;
};

class SettingsRegistry {
public:
    static SettingsRegistry& Instance() {
        static SettingsRegistry instance;
        return instance;
    }

    void RegisterSetting(const SettingDefinition& definition);
    void SetValue(const std::string& name, const std::any& value);
    
    template<typename T>
    void SetValue(const std::string& name, const T& value) {
        SetValue(name, std::any(value));
    }
    
    std::any GetValue(const std::string& name) const;
    bool LoadFromJson(const nlohmann::json& j);
    nlohmann::json SaveToJson() const;
    
    template<typename T>
    T GetValueAs(const std::string& name) const {
        return std::any_cast<T>(GetValue(name));
    }

private:
    SettingsRegistry() = default;
    std::unordered_map<std::string, SettingDefinition> m_definitions;
    std::unordered_map<std::string, std::any> m_values;
};