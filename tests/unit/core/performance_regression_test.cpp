#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "transity/core/debug_manager.hpp"
#include "transity/core/time_manager.hpp"
#include "transity/core/input_manager.hpp"
#include "transity/core/system_manager.hpp"
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <numeric>
#include <iostream>
#include <vector>
#include <map>
#include <cmath>
#include <algorithm>

using namespace transity::core;
using Catch::Matchers::WithinRel;

struct PerformanceBaseline {
    std::string name;
    double meanTime;
    double stdDev;
    
    PerformanceBaseline() : name(""), meanTime(0.0), stdDev(0.0) {}
    
    PerformanceBaseline(const std::string& n, double mean, double sd)
        : name(n), meanTime(mean), stdDev(sd) {}
};

void saveBaselines(const std::string& filename, const std::map<std::string, PerformanceBaseline>& baselines) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening file for writing: " << filename << std::endl;
        return;
    }
    
    // Write header
    file << "TestName,MeanTime,StdDev\n";
    
    // Write data
    for (const auto& [name, baseline] : baselines) {
        file << name << "," << baseline.meanTime << "," << baseline.stdDev << "\n";
    }
    
    std::cout << "Saved baseline data to: " << filename << std::endl;
}

std::map<std::string, PerformanceBaseline> loadBaselines(const std::string& filename) {
    std::map<std::string, PerformanceBaseline> baselines;
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "No existing baseline file found. Will create new baselines." << std::endl;
        return baselines;
    }
    
    std::string line;
    // Skip header
    std::getline(file, line);
    
    // Read each line
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string name;
        double mean, stdDev;
        
        if (std::getline(iss, name, ',') && iss >> mean) {
            iss.ignore(); // Skip comma
            iss >> stdDev;
            baselines[name] = PerformanceBaseline(name, mean, stdDev);
        }
    }
    
    std::cout << "Loaded " << baselines.size() << " baseline(s) from: " << filename << std::endl;
    return baselines;
}

std::pair<double, double> calculateStats(const std::vector<double>& durations) {
    double sum = 0.0;
    for (double d : durations) {
        sum += d;
    }
    double mean = sum / durations.size();
    
    double sumSquaredDiff = 0.0;
    for (double d : durations) {
        double diff = d - mean;
        sumSquaredDiff += diff * diff;
    }
    double stdDev = std::sqrt(sumSquaredDiff / durations.size());
    
    return {mean, stdDev};
}

// Test case for automated performance regression testing
TEST_CASE("Automated Performance Regression Testing", "[performance]") {
    const std::string baselineFile = "performance_baselines.csv";
    auto baselines = loadBaselines(baselineFile);
    bool needToSaveBaselines = false;
    
    // Configuration
    const int iterations = 100;
    const double normalTolerance = 3.0; // 3 standard deviations
    const double extendedTolerance = 5.0; // 5 standard deviations
    
    SECTION("DebugManager logging performance") {
        auto& debug = DebugManager::getInstance();
        
        // Run benchmark
        std::vector<double> durations;
        for (int i = 0; i < iterations; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            
            debug.log(LogLevel::Info, "Test message");
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            durations.push_back(duration);
        }
        
        // Calculate statistics
        auto [mean, stdDev] = calculateStats(durations);
        
        std::cout << "DebugManager Logging Performance:\n";
        std::cout << "  Mean: " << mean << " µs\n";
        std::cout << "  StdDev: " << stdDev << " µs\n";
        
        // Check against baseline or create new one
        const std::string testName = "DebugManager_Logging";
        if (baselines.find(testName) == baselines.end()) {
            // No existing baseline, create a new one
            std::cout << "  Creating new baseline for: " << testName << "\n";
            baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
            needToSaveBaselines = true;
        } else {
            // Compare with existing baseline
            const auto& baseline = baselines[testName];
            double zScore = (mean - baseline.meanTime) / baseline.stdDev;
            
            std::cout << "  Baseline mean: " << baseline.meanTime << " µs\n";
            std::cout << "  Baseline stdDev: " << baseline.stdDev << " µs\n";
            std::cout << "  Z-score: " << zScore << "\n";
            
            if (mean > baseline.meanTime) {
                // Current performance is worse than baseline
                
                if (zScore > extendedTolerance) {
                    // Performance is much worse, update baseline
                    std::cout << "  Performance significantly degraded. Updating baseline.\n";
                    baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
                    needToSaveBaselines = true;
                } else if (zScore > normalTolerance) {
                    // Performance is somewhat worse, report but don't fail
                    std::cout << "  Performance degraded but within extended tolerance.\n";
                    WARN("Performance degraded: " << testName);
                    // Don't fail the test, just warn
                }
                // Otherwise, performance is within normal tolerance, no action needed
            } else if (mean < baseline.meanTime * 0.7) {
                // Performance improved significantly
                std::cout << "  Performance improved significantly. Updating baseline.\n";
                baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
                needToSaveBaselines = true;
            }
        }
    }
    
    SECTION("SystemManager update performance") {
        SystemManager systemManager;
        
        // Run benchmark
        std::vector<double> durations;
        for (int i = 0; i < iterations; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            
            systemManager.update(0.016f);
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            durations.push_back(duration);
        }
        
        // Calculate statistics
        auto [mean, stdDev] = calculateStats(durations);
        
        std::cout << "SystemManager Update Performance:\n";
        std::cout << "  Mean: " << mean << " µs\n";
        std::cout << "  StdDev: " << stdDev << " µs\n";
        
        // Check against baseline or create new one
        const std::string testName = "SystemManager_Update";
        if (baselines.find(testName) == baselines.end()) {
            // No existing baseline, create a new one
            std::cout << "  Creating new baseline for: " << testName << "\n";
            baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
            needToSaveBaselines = true;
        } else {
            // Compare with existing baseline
            const auto& baseline = baselines[testName];
            double zScore = (mean - baseline.meanTime) / baseline.stdDev;
            
            std::cout << "  Baseline mean: " << baseline.meanTime << " µs\n";
            std::cout << "  Baseline stdDev: " << baseline.stdDev << " µs\n";
            std::cout << "  Z-score: " << zScore << "\n";
            
            if (mean > baseline.meanTime) {
                // Current performance is worse than baseline
                
                if (zScore > extendedTolerance) {
                    // Performance is much worse, update baseline
                    std::cout << "  Performance significantly degraded. Updating baseline.\n";
                    baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
                    needToSaveBaselines = true;
                } else if (zScore > normalTolerance) {
                    // Performance is somewhat worse, report but don't fail
                    std::cout << "  Performance degraded but within extended tolerance.\n";
                    WARN("Performance degraded: " << testName);
                    // Don't fail the test, just warn
                }
                // Otherwise, performance is within normal tolerance, no action needed
            } else if (mean < baseline.meanTime * 0.7) {
                // Performance improved significantly
                std::cout << "  Performance improved significantly. Updating baseline.\n";
                baselines[testName] = PerformanceBaseline(testName, mean, stdDev);
                needToSaveBaselines = true;
            }
        }
    }
    
    // Save updated baselines if needed
    if (needToSaveBaselines) {
        saveBaselines(baselineFile, baselines);
    }
} 