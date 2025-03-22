# Windows setup script for Transity
# Run this script as administrator

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Error "Please run this script as administrator"
    exit 1
}

# Install Chocolatey if not installed
if (-not (Get-Command choco -ErrorAction SilentlyContinue)) {
    Set-ExecutionPolicy Bypass -Scope Process -Force
    [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072
    Invoke-Expression ((New-Object System.Net.WebClient).DownloadString('https://chocolatey.org/install.ps1'))
}

# Install dependencies using Chocolatey
choco install -y `
    cmake `
    ninja `
    visualstudio2022-workload-nativedesktop `
    vcredist140 `
    git

# Install vcpkg if not already installed
$vcpkgPath = "C:\vcpkg"
if (-not (Test-Path $vcpkgPath)) {
    git clone https://github.com/Microsoft/vcpkg.git $vcpkgPath
    & $vcpkgPath\bootstrap-vcpkg.bat
}

# Install required packages with vcpkg
& $vcpkgPath\vcpkg install `
    sfml:x64-windows `
    imgui:x64-windows `
    nlohmann-json:x64-windows `
    spdlog:x64-windows `
    entt:x64-windows `
    catch2:x64-windows

# Integrate vcpkg with Visual Studio
& $vcpkgPath\vcpkg integrate install

Write-Host "Setup complete! Please restart your computer to ensure all changes take effect." 