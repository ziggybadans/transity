add_executable(unit_tests
    unit/main.cpp
    unit/core/application_test.cpp
    unit/core/input_manager_test.cpp
    unit/core/time_manager_test.cpp
)

target_link_libraries(unit_tests
    PRIVATE
        transity_core
        Catch2::Catch2WithMain
)

add_executable(integration_tests
    integration/main.cpp
    integration/simulation_test.cpp
)

target_link_libraries(integration_tests
    PRIVATE
        transity_core
        Catch2::Catch2WithMain
)

include(CTest)
include(Catch)
catch_discover_tests(unit_tests)
catch_discover_tests(integration_tests) 