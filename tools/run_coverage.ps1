# Transity Code Coverage Script for Windows
# This script runs the tests with coverage instrumentation and generates a coverage report

# Stop on first error
$ErrorActionPreference = "Stop"

# Configuration
$buildDir = "build_coverage"
$reportDir = "coverage_report"
$coverageOutputDir = "$reportDir/data"
$buildType = "Debug"
$genHtmlReport = $true

Write-Host "Starting Transity Code Coverage Analysis..." -ForegroundColor Cyan

# Create necessary directories
if (-not (Test-Path $reportDir)) {
    New-Item -ItemType Directory -Path $reportDir | Out-Null
}

if (-not (Test-Path $coverageOutputDir)) {
    New-Item -ItemType Directory -Path $coverageOutputDir | Out-Null
}

# Configure CMake with coverage flags
Write-Host "Configuring CMake with coverage instrumentation..." -ForegroundColor Yellow
cmake -S . -B $buildDir -DCMAKE_BUILD_TYPE=$buildType -DCMAKE_CXX_FLAGS="--coverage" -DCMAKE_EXE_LINKER_FLAGS="--coverage"

# Build the project
Write-Host "Building the project..." -ForegroundColor Yellow
cmake --build $buildDir --config $buildType

# Run unit tests
Write-Host "Running unit tests..." -ForegroundColor Yellow
Push-Location $buildDir
ctest -C $buildType --output-on-failure
if ($LASTEXITCODE -ne 0) {
    Write-Host "Error: Tests failed." -ForegroundColor Red
    exit 1
}
Pop-Location

# Collect coverage data
Write-Host "Collecting coverage data..." -ForegroundColor Yellow

# Get source files
$sourceFiles = Get-ChildItem -Path "src" -Filter "*.cpp" -Recurse
$sourceFiles += Get-ChildItem -Path "include" -Filter "*.hpp" -Recurse

# Run gcov on each source file
foreach ($file in $sourceFiles) {
    $relativePath = $file.FullName.Replace((Get-Location).Path + "\", "")
    Write-Host "Processing $relativePath" -ForegroundColor Gray
    
    $objDir = Join-Path $buildDir "CMakeFiles" 
    
    # Find the object file directory (it's nested under CMakeFiles)
    $objDirs = Get-ChildItem -Path $objDir -Directory -Recurse | Where-Object { 
        Test-Path (Join-Path $_.FullName ($file.Name -replace '\.(cpp|hpp)$', '.obj'))
    }
    
    foreach ($dir in $objDirs) {
        $objFile = Join-Path $dir.FullName ($file.Name -replace '\.(cpp|hpp)$', '.obj')
        if (Test-Path $objFile) {
            # Run gcov to generate coverage data
            gcov -o $dir.FullName $file.FullName
            
            # Move generated .gcov files to the coverage output directory
            $gcovFiles = Get-ChildItem -Path "." -Filter "*.gcov"
            foreach ($gcovFile in $gcovFiles) {
                Move-Item $gcovFile.FullName (Join-Path $coverageOutputDir $gcovFile.Name) -Force
            }
        }
    }
}

# Build the coverage report tool
Write-Host "Building coverage report tool..." -ForegroundColor Yellow
cmake --build $buildDir --config $buildType --target coverage_report

# Generate HTML report
if ($genHtmlReport) {
    Write-Host "Generating HTML coverage report..." -ForegroundColor Yellow
    & "./$buildDir/tools/coverage_report" $coverageOutputDir "$reportDir/coverage_report.html"
    
    if ($LASTEXITCODE -eq 0) {
        Write-Host "Coverage report generated successfully at: $reportDir/coverage_report.html" -ForegroundColor Green
        
        # Open the report in the default browser
        Start-Process "$reportDir/coverage_report.html"
    } else {
        Write-Host "Error generating coverage report." -ForegroundColor Red
    }
}

Write-Host "Code coverage analysis completed." -ForegroundColor Cyan 