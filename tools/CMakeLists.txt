# Tools CMakeLists.txt

# Coverage report generator
add_executable(coverage_report
    coverage_report.cpp
)

target_compile_features(coverage_report PRIVATE cxx_std_17)

# Install the coverage report tool
install(TARGETS coverage_report
    RUNTIME DESTINATION bin
) 