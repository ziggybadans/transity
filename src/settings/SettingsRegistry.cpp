#include "SettingsRegistry.h"
#include "../Debug.h"
#include <SFML/Graphics.hpp>

void SettingsRegistry::RegisterSetting(const SettingDefinition& definition) {
    m_definitions[definition.name] = definition;
    m_values[definition.name] = definition.defaultValue;
}

void SettingsRegistry::SetValue(const std::string& name, const std::any& value) {
    auto it = m_definitions.find(name);
    if (it == m_definitions.end()) {
        DEBUG_ERROR("Attempting to set unknown setting: ", name);
        return;
    }

    if (it->second.validator && !it->second.validator(value)) {
        DEBUG_ERROR("Invalid value for setting: ", name);
        return;
    }

    m_values[name] = value;
    
    if (it->second.onChange) {
        it->second.onChange(value);
    }
}

std::any SettingsRegistry::GetValue(const std::string& name) const {
    auto it = m_values.find(name);
    if (it == m_values.end()) {
        DEBUG_ERROR("Attempting to get unknown setting: ", name);
        return std::any();
    }
    return it->second;
}

bool SettingsRegistry::LoadFromJson(const nlohmann::json& j) {
    try {
        for (const auto& [category, settings] : j.items()) {
            for (const auto& [name, value] : settings.items()) {
                std::string fullName = category + "." + name;
                auto it = m_definitions.find(fullName);
                if (it == m_definitions.end()) continue;

                switch (it->second.type) {
                    case SettingType::Integer:
                        SetValue(fullName, static_cast<unsigned int>(value.get<int>()));
                        break;
                    case SettingType::Float:
                        SetValue(fullName, value.get<float>());
                        break;
                    case SettingType::Boolean:
                        SetValue(fullName, value.get<bool>());
                        break;
                    case SettingType::String:
                        SetValue(fullName, value.get<std::string>());
                        break;
                    case SettingType::Vector2u:
                        if (value.is_array() && value.size() == 2) {
                            SetValue(fullName, sf::Vector2u(
                                static_cast<unsigned int>(value[0].get<int>()),
                                static_cast<unsigned int>(value[1].get<int>())
                            ));
                        }
                        break;
                }
            }
        }
        return true;
    } catch (const std::exception& e) {
        DEBUG_ERROR("Error loading settings: ", e.what());
        return false;
    }
}

nlohmann::json SettingsRegistry::SaveToJson() const {
    nlohmann::json j;
    
    for (const auto& [name, def] : m_definitions) {
        size_t dot = name.find('.');
        if (dot == std::string::npos) continue;
        
        std::string category = name.substr(0, dot);
        std::string settingName = name.substr(dot + 1);
        
        try {
            const auto& value = m_values.at(name);
            
            switch (def.type) {
                case SettingType::Vector2u: {
                    if (const auto* vec = std::any_cast<sf::Vector2u>(&value)) {
                        j[category][settingName] = {vec->x, vec->y};
                    }
                    break;
                }
                case SettingType::Integer: {
                    if (const auto* val = std::any_cast<int>(&value)) {
                        j[category][settingName] = *val;
                    } else if (const auto* val = std::any_cast<unsigned int>(&value)) {
                        j[category][settingName] = static_cast<int>(*val);
                    }
                    break;
                }
                case SettingType::Float: {
                    if (const auto* val = std::any_cast<float>(&value)) {
                        j[category][settingName] = *val;
                    }
                    break;
                }
                case SettingType::Boolean: {
                    if (const auto* val = std::any_cast<bool>(&value)) {
                        j[category][settingName] = *val;
                    }
                    break;
                }
                case SettingType::String: {
                    if (const auto* val = std::any_cast<std::string>(&value)) {
                        j[category][settingName] = *val;
                    }
                    break;
                }
            }
        } catch (const std::exception& e) {
            DEBUG_WARNING("Failed to save setting ", name, ": ", e.what());
            continue;
        }
    }
    
    return j;
}