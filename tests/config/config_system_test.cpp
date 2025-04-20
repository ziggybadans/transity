#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

#include "config/ConfigSystem.hpp"

class ConfigSystemFileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary TOML file content
        std::ofstream outFile("temp_config.toml");
        outFile << "[General]\n";
        outFile << "appName = \"Transity Test\"\n";
        outFile << "logLevel = 3\n"; // Example int
        outFile << "\n";
        outFile << "[Graphics]\n";
        outFile << "fullscreen = false\n"; // Example bool
        outFile << "resolutionWidth = 1024\n";
        outFile << "windowWidth = 1920\n";
        outFile.close(); // Ensure file is written
    }

    void TearDown() override {
        // Remove the temporary config file after testing
        std::filesystem::remove("temp_config.toml");
    }
};

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

TEST_F(ConfigSystemFileTest, PrimaryFileParsedSuccess) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize("temp_config.toml");

    ASSERT_EQ(configSystem.getValue<std::string>("General.appName", std::string("")), "Transity Test");
    ASSERT_EQ(configSystem.getValue<int>("General.logLevel", 0), 3);
    ASSERT_EQ(configSystem.getValue<bool>("Graphics.fullscreen", false), false);
    ASSERT_EQ(configSystem.getValue<int>("Graphics.resolutionWidth", 0), 1024);
}

TEST_F(ConfigSystemFileTest, PrimaryFileOverridesDefaults) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize("temp_config.toml");

    ASSERT_EQ(configSystem.getValue<int>("Graphics.windowWidth", 0), 1920);
    ASSERT_EQ(configSystem.getValue<int>("windowWidth", 0), 800);
}

TEST_F(ConfigSystemFileTest, PrimaryFileParsedError) {
    transity::config::ConfigSystem configSystem;
    const std::string invalidPath = "invalid_config.toml";
    std::ofstream outFile(invalidPath);
    outFile << "[General\n";
    outFile << "appName = \"Test App\"\n";
    outFile.close();

    configSystem.initialize(invalidPath);

    ASSERT_EQ(configSystem.getValue<int>("windowWidth", 0), 800);
    ASSERT_EQ(configSystem.getValue<int>("windowHeight", 0), 600);
    ASSERT_EQ(configSystem.getValue<std::string>("windowTitle", std::string("")), "Transity");
    ASSERT_NE(configSystem.getValue<std::string>("General.appName", std::string("")), "Test App");
    ASSERT_EQ(configSystem.getValue<std::string>("General.appName", std::string("default_app")), "default_app");

    std::filesystem::remove(invalidPath);
}

TEST_F(ConfigSystemFileTest, UserFileOverridesPrimary) {
    transity::config::ConfigSystem configSystem;
    const std::string primaryPath = "temp_primary.toml";
    std::ofstream primaryFile(primaryPath);
    primaryFile << "[Graphics]\nWindowWidth = 1920\n";
    primaryFile.close();

    const std::string userPath = "temp_user.toml";
    std::ofstream userFile(userPath);
    userFile << "[Graphics]\nWindowWidth = 1280\n";
    userFile.close();

    configSystem.initialize(primaryPath, userPath);

    ASSERT_EQ(configSystem.getValue<int>("Graphics.WindowWidth", 0), 1280);

    std::filesystem::remove(primaryPath);
    std::filesystem::remove(userPath);
}

TEST(ConfigSystemTest, GetStringWrapper) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize();

    ASSERT_EQ(configSystem.getString("windowTitle"), "Transity");
    ASSERT_EQ(configSystem.getString("windowTitle", "fallback"), "Transity");
    ASSERT_EQ(configSystem.getString("nonExistentString"), "");
    ASSERT_EQ(configSystem.getString("nonExistentString", "fallback"), "fallback");
}

TEST(ConfigSystemTest, SetValueRuntime) {
    transity::config::ConfigSystem configSystem;
    configSystem.initialize();

    configSystem.setValue("windowWidth", 1024);
    ASSERT_EQ(configSystem.getValue<int>("windowWidth", 0), 1024);

    std::string newKey = "NewSetting.TestValue";
    bool newValue = true;
    configSystem.setValue<bool>(newKey, newValue);
    ASSERT_EQ(configSystem.getBool(newKey), newValue);
}

TEST(ConfigSystemTest, ShutdownSave) {
    const std::string primaryPath = "shutdown_primary.toml";
    std::ofstream primaryFile(primaryPath);
    primaryFile << "setting1 = 10\nsetting2 = 20\n";
    primaryFile.close();

    const std::string userPath = "shutdown_user.toml";
    std::ofstream userFile(userPath);
    userFile << "setting2 = 200\nsetting3 = 300\n";
    userFile.close();

    transity::config::ConfigSystem configSystem;
    configSystem.initialize(primaryPath, userPath);

    configSystem.setValue<int>("setting3", 3000);
    configSystem.setValue<std::string>("setting4", "runtimeValue");

    configSystem.shutdown();

    ASSERT_TRUE(std::filesystem::exists(primaryPath));
    try {
        toml::table savedTable = toml::parse_file(userPath);

        ASSERT_FALSE(savedTable.contains("setting1"));
        ASSERT_EQ(savedTable["setting2"].value_or(0), 200);
        ASSERT_EQ(savedTable["setting3"].value_or(0), 3000);
        std::optional<std::string> setting4_opt = savedTable["setting4"].value<std::string>();
        ASSERT_EQ(*setting4_opt, "runtimeValue");
    } catch (const std::exception& e) {
        FAIL() << "Failed to parse saved user config: " << e.what();
    }

    std::filesystem::remove(primaryPath);
    std::filesystem::remove(userPath);
}