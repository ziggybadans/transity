#include <gtest/gtest.h>

#include "config/ConfigSystem.h"

TEST(ConfigSystemTest, DefaultsLoaded) {
    transity::config::ConfigSystem configSystem;
    
    configSystem.initialize();

    EXPECT_EQ(configSystem.getValue("windowWidth", 0), 800);
    EXPECT_EQ(configSystem.getValue("windowHeight", 0), 600);
    EXPECT_EQ(configSystem.getValue("windowTitle", std::string("")), "Transity");
}