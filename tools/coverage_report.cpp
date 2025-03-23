#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <filesystem>
#include <algorithm>
#include <iomanip>

namespace fs = std::filesystem;

// Structure to hold coverage data for a single file
struct FileCoverage {
    std::string filename;
    size_t totalLines{0};
    size_t coveredLines{0};
    std::vector<int> uncoveredLines;
    double coverage{0.0};
};

// Structure to hold coverage data for a module/component
struct ModuleCoverage {
    std::string name;
    std::vector<FileCoverage> files;
    size_t totalLines{0};
    size_t coveredLines{0};
    double coverage{0.0};
};

// Parse gcov data file and extract coverage information
FileCoverage parseGcovFile(const std::string& filepath) {
    FileCoverage result;
    result.filename = fs::path(filepath).stem().string();
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return result;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip header lines
        if (lineNumber <= 5) continue;
        
        // Check if line contains coverage data
        if (line.find("    #####:") != std::string::npos) {
            // Extract line number of uncovered line
            size_t pos = line.find(':');
            if (pos != std::string::npos) {
                try {
                    int uncoveredLine = std::stoi(line.substr(pos + 1));
                    result.uncoveredLines.push_back(uncoveredLine);
                    result.totalLines++;
                } catch (const std::exception& e) {
                    // Skip if not a valid line number
                }
            }
        } else if (line.find("        -:") == std::string::npos && 
                  line.find("function") == std::string::npos) {
            // Count covered lines (not comments or non-executable lines)
            result.totalLines++;
            result.coveredLines++;
        }
    }
    
    // Calculate coverage percentage
    if (result.totalLines > 0) {
        result.coverage = static_cast<double>(result.coveredLines) / result.totalLines * 100.0;
    }
    
    return result;
}

// Group files by module/component based on directory structure
std::vector<ModuleCoverage> groupByModule(const std::vector<FileCoverage>& files) {
    std::map<std::string, ModuleCoverage> moduleMap;
    
    for (const auto& file : files) {
        // Extract module name from file path (assuming src/module/file.cpp structure)
        std::string filename = file.filename;
        size_t pos = filename.find('_');
        std::string moduleName;
        
        if (pos != std::string::npos) {
            moduleName = filename.substr(0, pos);
        } else {
            moduleName = "core"; // Default module name
        }
        
        // Add file to corresponding module
        if (moduleMap.find(moduleName) == moduleMap.end()) {
            moduleMap[moduleName] = {moduleName};
        }
        
        moduleMap[moduleName].files.push_back(file);
        moduleMap[moduleName].totalLines += file.totalLines;
        moduleMap[moduleName].coveredLines += file.coveredLines;
    }
    
    // Calculate coverage percentage for each module
    for (auto& [name, module] : moduleMap) {
        if (module.totalLines > 0) {
            module.coverage = static_cast<double>(module.coveredLines) / module.totalLines * 100.0;
        }
    }
    
    // Convert map to vector
    std::vector<ModuleCoverage> result;
    for (const auto& [name, module] : moduleMap) {
        result.push_back(module);
    }
    
    // Sort modules by coverage (ascending)
    std::sort(result.begin(), result.end(), 
              [](const ModuleCoverage& a, const ModuleCoverage& b) {
                  return a.coverage < b.coverage;
              });
    
    return result;
}

// Generate HTML coverage report
void generateHtmlReport(const std::vector<ModuleCoverage>& modules, const std::string& outputPath) {
    std::ofstream html(outputPath);
    if (!html.is_open()) {
        std::cerr << "Error: Could not create HTML report at " << outputPath << std::endl;
        return;
    }
    
    // Calculate overall coverage
    size_t totalLines = 0;
    size_t coveredLines = 0;
    
    for (const auto& module : modules) {
        totalLines += module.totalLines;
        coveredLines += module.coveredLines;
    }
    
    double overallCoverage = 0.0;
    if (totalLines > 0) {
        overallCoverage = static_cast<double>(coveredLines) / totalLines * 100.0;
    }
    
    // Write HTML header
    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "    <meta charset=\"UTF-8\">\n"
         << "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
         << "    <title>Transity Test Coverage Report</title>\n"
         << "    <style>\n"
         << "        body { font-family: Arial, sans-serif; margin: 20px; }\n"
         << "        h1, h2 { color: #333; }\n"
         << "        .summary { margin-bottom: 30px; }\n"
         << "        .module { margin-bottom: 20px; border: 1px solid #ddd; padding: 10px; border-radius: 5px; }\n"
         << "        .module-header { display: flex; justify-content: space-between; align-items: center; }\n"
         << "        .file { margin: 10px 0; padding: 5px; background-color: #f5f5f5; }\n"
         << "        .progress-bar { height: 20px; background-color: #e0e0e0; border-radius: 10px; overflow: hidden; }\n"
         << "        .progress { height: 100%; }\n"
         << "        .good { background-color: #4CAF50; }\n"
         << "        .warning { background-color: #FFC107; }\n"
         << "        .poor { background-color: #F44336; }\n"
         << "        .details { margin-top: 5px; font-size: 0.9em; color: #666; }\n"
         << "        .uncovered { color: #F44336; margin-top: 5px; font-size: 0.9em; }\n"
         << "    </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "    <h1>Transity Test Coverage Report</h1>\n"
         << "    <div class=\"summary\">\n"
         << "        <h2>Overall Coverage: " << std::fixed << std::setprecision(2) << overallCoverage << "%</h2>\n"
         << "        <div class=\"progress-bar\">\n"
         << "            <div class=\"progress " << (overallCoverage >= 80.0 ? "good" : (overallCoverage >= 60.0 ? "warning" : "poor")) << "\" "
         << "                 style=\"width: " << overallCoverage << "%;\">\n"
         << "            </div>\n"
         << "        </div>\n"
         << "        <div class=\"details\">\n"
         << "            Total Lines: " << totalLines << ", Covered Lines: " << coveredLines << "\n"
         << "        </div>\n"
         << "    </div>\n";
    
    // Write module sections
    for (const auto& module : modules) {
        html << "    <div class=\"module\">\n"
             << "        <div class=\"module-header\">\n"
             << "            <h2>" << module.name << "</h2>\n"
             << "            <span>" << std::fixed << std::setprecision(2) << module.coverage << "%</span>\n"
             << "        </div>\n"
             << "        <div class=\"progress-bar\">\n"
             << "            <div class=\"progress " << (module.coverage >= 80.0 ? "good" : (module.coverage >= 60.0 ? "warning" : "poor")) << "\" "
             << "                 style=\"width: " << module.coverage << "%;\">\n"
             << "            </div>\n"
             << "        </div>\n"
             << "        <div class=\"details\">\n"
             << "            Total Lines: " << module.totalLines << ", Covered Lines: " << module.coveredLines << "\n"
             << "        </div>\n";
        
        // Write file sections
        for (const auto& file : module.files) {
            html << "        <div class=\"file\">\n"
                 << "            <div class=\"file-header\">\n"
                 << "                <strong>" << file.filename << "</strong>\n"
                 << "                <span>" << std::fixed << std::setprecision(2) << file.coverage << "%</span>\n"
                 << "            </div>\n"
                 << "            <div class=\"progress-bar\">\n"
                 << "                <div class=\"progress " << (file.coverage >= 80.0 ? "good" : (file.coverage >= 60.0 ? "warning" : "poor")) << "\" "
                 << "                     style=\"width: " << file.coverage << "%;\">\n"
                 << "                </div>\n"
                 << "            </div>\n"
                 << "            <div class=\"details\">\n"
                 << "                Total Lines: " << file.totalLines << ", Covered Lines: " << file.coveredLines << "\n"
                 << "            </div>\n";
            
            // Write uncovered lines if any
            if (!file.uncoveredLines.empty()) {
                html << "            <div class=\"uncovered\">\n"
                     << "                Uncovered Lines: ";
                
                size_t maxLinesToShow = 10;
                for (size_t i = 0; i < std::min(file.uncoveredLines.size(), maxLinesToShow); ++i) {
                    html << file.uncoveredLines[i];
                    if (i < std::min(file.uncoveredLines.size(), maxLinesToShow) - 1) {
                        html << ", ";
                    }
                }
                
                if (file.uncoveredLines.size() > maxLinesToShow) {
                    html << ", ... (" << (file.uncoveredLines.size() - maxLinesToShow) << " more)";
                }
                
                html << "\n            </div>\n";
            }
            
            html << "        </div>\n";
        }
        
        html << "    </div>\n";
    }
    
    // Write HTML footer
    html << "</body>\n"
         << "</html>\n";
    
    std::cout << "Coverage report generated at: " << outputPath << std::endl;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <coverage_data_dir> <output_file.html>" << std::endl;
        return 1;
    }
    
    std::string coverageDir = argv[1];
    std::string outputPath = argv[2];
    
    // Check if coverage directory exists
    if (!fs::exists(coverageDir) || !fs::is_directory(coverageDir)) {
        std::cerr << "Error: Coverage data directory does not exist: " << coverageDir << std::endl;
        return 1;
    }
    
    // Parse coverage data files
    std::vector<FileCoverage> files;
    for (const auto& entry : fs::directory_iterator(coverageDir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".gcov") {
            FileCoverage fileCoverage = parseGcovFile(entry.path().string());
            files.push_back(fileCoverage);
        }
    }
    
    if (files.empty()) {
        std::cerr << "Error: No coverage data files found in directory: " << coverageDir << std::endl;
        return 1;
    }
    
    // Group files by module
    std::vector<ModuleCoverage> modules = groupByModule(files);
    
    // Generate HTML report
    generateHtmlReport(modules, outputPath);
    
    return 0;
} 