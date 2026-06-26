#pragma once

#include <cstdlib>
#include <filesystem>
#include <string>

namespace agent_tui {

class KnownPaths {
public:
    static std::filesystem::path home() {
#ifdef _WIN32
        const char* value = std::getenv("USERPROFILE");
#else
        const char* value = std::getenv("HOME");
#endif
        if (value == nullptr || std::string(value).empty()) {
            return std::filesystem::current_path();
        }
        return std::filesystem::path{value};
    }

    static std::filesystem::path desktop() { return home() / "Desktop"; }
    static std::filesystem::path downloads() { return home() / "Downloads"; }
    static std::filesystem::path documents() { return home() / "Documents"; }
    static std::filesystem::path pictures() { return home() / "Pictures"; }

    static std::filesystem::path resolve_alias(const std::string& value) {
        if (value == "~" || value == "home") {
            return home();
        }
        if (value == "desktop" || value == "Desktop") {
            return desktop();
        }
        if (value == "downloads" || value == "Downloads") {
            return downloads();
        }
        if (value == "documents" || value == "Documents") {
            return documents();
        }
        if (value == "pictures" || value == "Pictures") {
            return pictures();
        }
        return std::filesystem::path{value};
    }
};

}  // namespace agent_tui
