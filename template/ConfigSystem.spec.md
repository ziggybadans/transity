// File: specs/ConfigSystem.spec.md
// Module: ConfigSystem
// Description: Handles loading and accessing application configuration settings.

# Specification: Configuration System

## 1. Overview

Provides a mechanism to load configuration settings from one or more sources (e.g., default values, configuration files) and allows other systems to retrieve these settings.

## 2. Dependencies

- (Potentially) `LoggingSystem`: To log information about configuration loading or errors.

## 3. Data Structures

- `ConfigSource`: Enum or identifier for the source of a setting (e.g., `DEFAULT`, `FILE_PRIMARY`, `FILE_USER`).
- `ConfigValue`: A variant type capable of holding different data types (e.g., `string`, `int`, `float`, `bool`, potentially lists or maps).
- `ConfigStore`: Internal data structure (e.g., a map or dictionary) storing configuration keys (strings) and their corresponding `ConfigValue`s.

## 4. Functions / Methods

### `initialize(defaultConfig, primaryConfigFilepath, userConfigFilepath)`

1.  **Load Defaults:** Populate the `ConfigStore` with hard-coded default values provided via `defaultConfig` (e.g., a map or struct). Mark these values with `ConfigSource::DEFAULT`.
    - `TDD_ANCHOR: Test_Config_Defaults_Loaded`
2.  **Load Primary Config File:**
    - Check if `primaryConfigFilepath` exists.
    - If yes, attempt to parse the file (e.g., INI, JSON, YAML format - TBD).
    - For each key-value pair read:
        - Convert the value to the appropriate `ConfigValue` type.
        - Overwrite any existing value for the same key in `ConfigStore`, updating its `ConfigSource` to `FILE_PRIMARY`.
    - Log information about loaded file or file not found.
    - Handle parsing errors gracefully (log error, potentially continue with defaults or exit).
    - `TDD_ANCHOR: Test_Config_PrimaryFile_NotFound`
    - `TDD_ANCHOR: Test_Config_PrimaryFile_Parsing_Success`
    - `TDD_ANCHOR: Test_Config_PrimaryFile_Parsing_Error`
    - `TDD_ANCHOR: Test_Config_PrimaryFile_Overrides_Defaults`
3.  **Load User Config File (Optional):**
    - Repeat step 2 for `userConfigFilepath`, if provided.
    - Values from the user file should override both defaults and primary file values. Update `ConfigSource` to `FILE_USER`.
    - `TDD_ANCHOR: Test_Config_UserFile_Overrides_Primary`
4.  **Log Initialization:** Log successful initialization and potentially the final configuration sources used.

### `getValue<T>(key, defaultValue)`

1.  **Lookup Key:** Search for `key` in the `ConfigStore`.
2.  **Found:**
    - If found, attempt to convert the stored `ConfigValue` to the requested type `T`.
    - If conversion is successful, return the converted value.
    - If conversion fails (e.g., requesting `int` for a `string` value "hello"), log a warning/error and return `defaultValue`.
    - `TDD_ANCHOR: Test_Config_GetValue_Found_CorrectType`
    - `TDD_ANCHOR: Test_Config_GetValue_Found_IncorrectType`
3.  **Not Found:**
    - Log a warning that the key was not found.
    - Return the provided `defaultValue`.
    - `TDD_ANCHOR: Test_Config_GetValue_NotFound`

### `getString(key, defaultValue)`, `getInt(key, defaultValue)`, etc. (Convenience Wrappers)

- Provide type-specific wrappers around `getValue<T>` for common types.
    - `TDD_ANCHOR: Test_Config_GetTypedValue_Wrappers`

### `setValue(key, value)` (Optional - For runtime changes/saving)

1.  **Update Store:** Add or update the `key` in `ConfigStore` with the new `value`.
2.  **Mark Source:** Potentially mark the source as `RUNTIME` or similar.
3.  **Persistence (Optional):** If saving configuration is required, this might trigger writing back to a user config file. (Likely out of scope for initial phase).
    - `TDD_ANCHOR: Test_Config_SetValue_Runtime`

### `shutdown()`

1.  **Save Configuration (Optional):** If runtime changes are saved, write the current configuration (potentially only user-level or runtime changes) back to the user config file.
    - `TDD_ANCHOR: Test_Config_Shutdown_Save`
2.  **Cleanup:** Release any resources (e.g., file handles if kept open, though unlikely).
3.  **Log Shutdown:** Log successful shutdown of the configuration system.

## 5. Configuration File Format

- **Decision Needed:** Choose a configuration file format (e.g., INI, JSON, YAML, TOML). Simplicity (like INI) might be suitable initially. The chosen format impacts parsing logic.

## 6. Edge Cases & Considerations

- **File Format Errors:** Robust handling of syntax errors in configuration files.
- **Type Mismatches:** How strictly to enforce types? Log errors or attempt conversions?
- **Missing Keys:** Consistent behaviour (using defaults, logging warnings) when requested keys don't exist.
- **Concurrency:** If configuration can be reloaded or modified at runtime, ensure thread safety for accessing the `ConfigStore`.
- **Secrets Management:** Configuration should *not* contain sensitive data like passwords or API keys directly. Define strategies for handling secrets if needed later (e.g., environment variables, separate secure storage).

## 7. TDD Anchors Summary

- `Test_Config_Defaults_Loaded`
- `Test_Config_PrimaryFile_NotFound`
- `Test_Config_PrimaryFile_Parsing_Success`
- `Test_Config_PrimaryFile_Parsing_Error`
- `Test_Config_PrimaryFile_Overrides_Defaults`
- `Test_Config_UserFile_Overrides_Primary`
- `Test_Config_GetValue_Found_CorrectType`
- `Test_Config_GetValue_Found_IncorrectType`
- `Test_Config_GetValue_NotFound`
- `Test_Config_GetTypedValue_Wrappers`
- `Test_Config_SetValue_Runtime` (Optional)
- `Test_Config_Shutdown_Save` (Optional)