#include <gtest/gtest.h>

#include "config/ConfigSystem.h"

TEST(ConfigSystemTest, DefaultsLoaded) {
    transity::config::ConfigSystem configSystem;
    
    configSystem.initialize();

    ASSERT_EQ(configSystem.getValue<int>("windowWidth", 0), 800);
    ASSERT_EQ(configSystem.getValue<int>("windowHeight", 0), 600);
    ASSERT_EQ(configSystem.getValue<std::string>("windowTitle", std::string("")), "Transity");
}

TEST(ConfigSystemTest, GetValueNotFound) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize();

    ASSERT_EQ(configSystem.getValue<int>("nonExistentKey", 0), 0);
    ASSERT_EQ(configSystem.getValue<std::string>("nonExistentKey", "fallback"), "fallback");
}

TEST(ConfigSystemTest, GetValueIncorrectType) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize();

    ASSERT_EQ(configSystem.getValue<std::string>("windowWidth", "type_mismatch"), "type_mismatch");
}

TEST(ConfigSystemTest, PrimaryFileNotFound) {
    transity::config::ConfigSystem configSystem;
    const std::string nonExistantPath = "non_existant_config.toml";

    configSystem.initialize(nonExistantPath);

    ASSERT_EQ(configSystem.getValue<int>("windowWidth", 0), 800);
    ASSERT_EQ(configSystem.getValue<int>("windowHeight", 0), 600);
    ASSERT_EQ(configSystem.getValue<std::string>("windowTitle", std::string("")), "Transity");
}