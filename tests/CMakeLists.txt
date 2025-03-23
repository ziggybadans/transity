add_executable(unit_tests
    unit/main.cpp
    unit/core/application_test.cpp
    unit/core/input_manager_test.cpp
    unit/core/time_manager_test.cpp
    unit/core/system_manager_test.cpp
    unit/core/debug_manager_test.cpp
    unit/core/performance_test.cpp
    unit/core/performance_regression_test.cpp
    unit/ecs/entity_manager_test.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        transity_core
        transity_ecs
        Catch2::Catch2WithMain
)

add_executable(integration_tests
    integration/main.cpp
    integration/simulation_test.cpp
    integration/core_systems_integration_test.cpp
)

target_link_libraries(integration_tests
    PRIVATE
        transity_core
        transity_ecs
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
catch_discover_tests(integration_tests) 