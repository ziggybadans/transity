#include <gtest/gtest.h>
#include <toml++/toml.hpp>
#include <filesystem>
#include <fstream>
#include <thread>
#include <vector>
#include <string>
#include <atomic>

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

TEST(ConfigSystemTest, GetIntValueFound) {
    // Arrange
    transity::config::ConfigSystem config;
    config.setValue("test.integer", 123); // Set a runtime value

    // Act
    int result = config.getInt("test.integer", 999); // Provide a different default

    // Assert
    ASSERT_EQ(result, 123); // Expect the value we set
}

TEST(ConfigSystemTest, GetIntValueNotFoundUseDefault) {
    // Arrange
    transity::config::ConfigSystem config;
    // Ensure "missing.integer" is not set anywhere

    // Act
    int result = config.getInt("missing.integer", 42); // Provide a default

    // Assert
    ASSERT_EQ(result, 42); // Expect the default value
}

TEST(ConfigSystemTest, GetBoolValueFound) {
    // Arrange
    transity::config::ConfigSystem config;
    config.setValue("test.boolean", true); // Set a runtime value

    // Act
    bool result = config.getBool("test.boolean", false); // Provide a different default

    // Assert
    ASSERT_EQ(result, true); // Expect the value we set
}

TEST(ConfigSystemTest, GetBoolValueNotFoundUseDefault) {
    // Arrange
    transity::config::ConfigSystem config;
    // Ensure "missing.boolean" is not set anywhere

    // Act
    bool result = config.getBool("missing.boolean", true); // Provide a default

    // Assert
    ASSERT_EQ(result, true); // Expect the default value
}

TEST(ConfigSystemTest, GetDoubleValueFound) {
    // Arrange
    transity::config::ConfigSystem config;
    config.setValue("test.double", 123.456); // Set a runtime value

    // Act
    double result = config.getDouble("test.double", 999.999); // Provide a different default

    // Assert
    ASSERT_DOUBLE_EQ(result, 123.456); // Expect the value we set (use ASSERT_DOUBLE_EQ for floating point)
}

TEST(ConfigSystemTest, GetDoubleValueNotFoundUseDefault) {
    // Arrange
    transity::config::ConfigSystem config;
    // Ensure "missing.double" is not set anywhere

    // Act
    double result = config.getDouble("missing.double", 42.42); // Provide a default

    // Assert
    ASSERT_DOUBLE_EQ(result, 42.42); // Expect the default value
}

TEST(ConfigSystemTest, GetFloatValueFound) {
    // Arrange
    transity::config::ConfigSystem config;
    // Note: setValue likely stores as double due to template deduction,
    // but getFloat should handle the conversion.
    config.setValue("test.float", 78.9f); // Set a runtime value

    // Act
    float result = config.getFloat("test.float", 99.9f); // Provide a different default

    // Assert
    ASSERT_FLOAT_EQ(result, 78.9f); // Expect the value we set (use ASSERT_FLOAT_EQ)
}

TEST(ConfigSystemTest, GetFloatValueNotFoundUseDefault) {
    // Arrange
    transity::config::ConfigSystem config;
    // Ensure "missing.float" is not set anywhere

    // Act
    float result = config.getFloat("missing.float", 12.3f); // Provide a default

    // Assert
    ASSERT_FLOAT_EQ(result, 12.3f); // Expect the default value
}

TEST(ConfigSystemTest, SetValueOverwrite) {
    // Arrange
    transity::config::ConfigSystem config;

    // Act
    config.setValue("runtime.value", 100);
    config.setValue("runtime.value", 200); // Overwrite

    // Assert
    ASSERT_EQ(config.getInt("runtime.value", 0), 200); // Check the final value
}

TEST(ConfigSystemTest, SetValueNestedConflict) {
    // Arrange
    transity::config::ConfigSystem config;
    config.setValue("a.b", 1); // Set a non-table value at a.b

    // Act
    config.setValue("a.b.c", 2); // Attempt to create a table under a.b

    // Assert
    ASSERT_EQ(config.getInt("a.b", 0), 1); // Original value should remain
    ASSERT_EQ(config.getInt("a.b.c", 999), 999); // Nested value should not exist (get default)
    // Implicitly asserts no crash occurred.
}

TEST(ConfigSystemTest, SetValueEmptyKey) {
    // Arrange
    transity::config::ConfigSystem config;
    // You might want to get the initial state/size of runtimeOverrides if possible,
    // but it's not strictly necessary for just checking graceful handling.

    // Act
    // The main point is that this call should not crash or throw.
    ASSERT_NO_THROW(config.setValue("", 123));

    // Assert
    // Optionally, verify no key named "" was actually added if your getValue allows empty keys (it likely doesn't based on splitPath)
    // Or verify the overall size/state didn't change unexpectedly.
    ASSERT_EQ(config.getInt("", 999), 999); // Assuming getInt handles empty key similarly
}

TEST(ConfigSystemTest, ShutdownNoUserPathNoSave) {
    // Arrange
    const std::string userFilePath = "non_existent_user_for_no_save_test.toml";
    std::filesystem::remove(userFilePath); // Ensure it doesn't exist beforehand

    transity::config::ConfigSystem config;
    // Initialize WITHOUT a user file path
    config.initialize("config.toml", ""); // Assuming config.toml might exist or defaults are fine

    config.setValue("some.runtime.setting", 12345); // Add a runtime value

    // Act
    config.shutdown();

    // Assert
    ASSERT_FALSE(std::filesystem::exists(userFilePath)); // Crucial: File should NOT have been created

    // Cleanup (optional, as it shouldn't exist, but safe)
    std::filesystem::remove(userFilePath);
}

TEST(ConfigSystemTest, ShutdownCreatesUserFileWithRuntimeValues) {
    // Arrange
    const std::string userFilePath = "created_on_shutdown.toml";
    std::filesystem::remove(userFilePath); // Ensure it doesn't exist

    transity::config::ConfigSystem config;
    // Initialize with a user file path that doesn't exist yet
    config.initialize("", userFilePath); // No primary file needed for this test

    // Set some runtime values
    config.setValue("runtime.string", "test_value");
    config.setValue("runtime.integer", 123);
    config.setValue("runtime.nested.boolean", true);

    // Act
    config.shutdown();

    // Assert
    // 1. Check if the file was created
    ASSERT_TRUE(std::filesystem::exists(userFilePath));

    // 2. Parse the created file and check its contents
    try {
        toml::table savedTable = toml::parse_file(userFilePath);

        // Check for the specific runtime values we set
        ASSERT_TRUE(savedTable.contains("runtime"));
        ASSERT_TRUE(savedTable["runtime"].is_table());
        auto* runtimeTable = savedTable["runtime"].as_table(); // Get pointer to table
        ASSERT_NE(runtimeTable, nullptr); // Ensure table exists

        // --- Revised String Check ---
        auto string_node = runtimeTable->get("string");
        ASSERT_NE(string_node, nullptr); // Check node exists
        ASSERT_TRUE(string_node->is_string()); // Check node type
        // Explicitly get value as std::string and compare
        ASSERT_EQ(string_node->value<std::string>().value_or(""), "test_value");

        // --- Revised Integer Check ---
        auto integer_node = runtimeTable->get("integer");
        ASSERT_NE(integer_node, nullptr);
        ASSERT_TRUE(integer_node->is_integer());
        // TOML integers are often 64-bit, be explicit or let value<> handle it
        ASSERT_EQ(integer_node->value<int>().value_or(0), 123);

        // --- Revised Nested Boolean Check ---
        auto nested_node = runtimeTable->get("nested");
        ASSERT_NE(nested_node, nullptr);
        ASSERT_TRUE(nested_node->is_table());
        auto* nestedTable = nested_node->as_table();
        ASSERT_NE(nestedTable, nullptr);

        auto boolean_node = nestedTable->get("boolean");
        ASSERT_NE(boolean_node, nullptr);
        ASSERT_TRUE(boolean_node->is_boolean());
        ASSERT_EQ(boolean_node->value<bool>().value_or(false), true);

        // Optionally, check that default values weren't saved
        ASSERT_FALSE(savedTable.contains("windowWidth"));

    } catch (const toml::parse_error& err) {
        FAIL() << "Failed to parse the created user config file: " << err.what();
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception parsing created user file: " << e.what();
    }

    // Cleanup
    std::filesystem::remove(userFilePath);
}

TEST(ConfigSystemTest, ShutdownMergesNestedKeys) {
    // Arrange
    const std::string userFilePath = "nested_merge_user.toml";

    // 1. Create initial user file with nested structure
    std::filesystem::remove(userFilePath); // Clean slate
    { // Scope for ofstream
        std::ofstream userFile(userFilePath);
        userFile << "[Graphics.Resolution]\n";
        userFile << "Width = 1920\n";
        userFile << "Height = 1080\n\n";
        userFile << "[Audio]\n";
        userFile << "GlobalMute = false\n";
        userFile.close();
        ASSERT_TRUE(std::filesystem::exists(userFilePath)); // Verify creation
    }


    transity::config::ConfigSystem config;
    // Initialize with the user file we just created
    config.initialize("", userFilePath);

    // 2. Set runtime values, some overlapping/nested differently
    config.setValue("Graphics.Resolution.RefreshRate", 144); // Add to existing table
    config.setValue("Audio.Volume.Master", 75);          // Create new nested table 'Volume'
    config.setValue("Input.Mouse.Sensitivity", 0.8);     // Create new top-level table 'Input'
    config.setValue("Audio.GlobalMute", true);           // Overwrite existing value

    // Act
    config.shutdown(); // Should save merged data back to userFilePath

    // Assert
    ASSERT_TRUE(std::filesystem::exists(userFilePath)); // File should still exist

    try {
        toml::table savedTable = toml::parse_file(userFilePath);

        // Verify Graphics section (original + added)
        ASSERT_TRUE(savedTable["Graphics"]["Resolution"]["Width"].value_or(0) == 1920);
        ASSERT_TRUE(savedTable["Graphics"]["Resolution"]["Height"].value_or(0) == 1080);
        ASSERT_TRUE(savedTable["Graphics"]["Resolution"]["RefreshRate"].value_or(0) == 144);

        // Verify Audio section (overwritten + new nested)
        ASSERT_TRUE(savedTable["Audio"]["GlobalMute"].value_or(false) == true);
        ASSERT_TRUE(savedTable["Audio"]["Volume"]["Master"].value_or(0) == 75);

        // Verify Input section (new)
        ASSERT_TRUE(savedTable["Input"]["Mouse"]["Sensitivity"].value_or(0.0) == 0.8); // Use floating point compare if needed

    } catch (const toml::parse_error& err) {
        FAIL() << "Failed to parse the merged user config file: " << err.what();
    } catch (const std::exception& e) {
        FAIL() << "Unexpected exception parsing merged user file: " << e.what();
    }

    // Cleanup
    std::filesystem::remove(userFilePath);
}

TEST(ConfigSystemTest, ConcurrentAccess) {
    transity::config::ConfigSystem configSystem;
    // Initialize if necessary, or rely on constructor init

    const int num_threads = 8;
    const int operations_per_thread = 1000;
    std::vector<std::thread> threads;

    // Pre-set a value to check later
    std::string testKey = "concurrency.test.value";
    int initialValue = 100;
    configSystem.setValue(testKey, initialValue);

    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&configSystem, i, testKey, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                // Mix of reads and writes
                if (j % 10 == 0) { // Occasionally write
                    std::string writeKey = "thread_" + std::to_string(i) + ".write_val";
                    configSystem.setValue(writeKey, i * 100 + j);
                } else if (j % 5 == 0) { // Read the test key
                    int val = configSystem.getInt(testKey, -1);
                    // Could add an assertion here if needed, but often just ensuring
                    // no crashes and checking at the end is sufficient for basic safety.
                    // ASSERT_NE(val, -1); // Example: Check it was found
                } else { // Read some other key
                    configSystem.getString("windowTitle", "Default");
                }
            }
            // Set a final value for the test key from some threads
            if (i % 2 == 0) {
                 configSystem.setValue(testKey, 200 + i);
            }
        });
    }

    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }

    // Verification after threads finish
    // Check if the final value of testKey is one of the values set by the writing threads
    int finalValue = configSystem.getInt(testKey, -1);
    bool foundExpectedValue = false;
    for (int i = 0; i < num_threads; ++i) {
        if (i % 2 == 0 && finalValue == (200 + i)) {
            foundExpectedValue = true;
            break;
        }
    }
     // If no even thread wrote last, it might still be the initial value
    if (!foundExpectedValue && finalValue == initialValue) {
         foundExpectedValue = true;
    }

    ASSERT_TRUE(foundExpectedValue); // Assert that the final value is one of the expected ones

    // You might also check some of the thread-specific keys
    int expected_last_j = ((operations_per_thread - 1) / 10) * 10;
    ASSERT_EQ(configSystem.getInt("thread_0.write_val", -1), expected_last_j); // Example check
}