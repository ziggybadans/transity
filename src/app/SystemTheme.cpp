#include "SystemTheme.h"
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>

#ifdef __APPLE__
Theme SystemTheme::getSystemTheme() {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("defaults read -g AppleInterfaceStyle", "r"), pclose);
    if (!pipe) {
        // Cannot open pipe, default to light theme
        return Theme::Light;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    if (result.find("Dark") != std::string::npos) {
        return Theme::Dark;
    }

    return Theme::Light;
}
#else
// Default implementation for non-Apple platforms
Theme SystemTheme::getSystemTheme() {
    return Theme::Light;
}
#endif