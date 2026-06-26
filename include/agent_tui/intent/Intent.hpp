#pragma once

#include <string>

namespace agent_tui {

enum class IntentType {
    Unknown,
    ListDir,
    ReadFile,
    SearchText,
    ConfigureProject,
    BuildProject,
    TestProject,
};

struct Intent {
    IntentType type = IntentType::Unknown;
    std::string argument;
    int confidence = 0;
    std::string reason;
};

inline std::string intent_type_name(IntentType type) {
    switch (type) {
        case IntentType::Unknown:
            return "unknown";
        case IntentType::ListDir:
            return "list_dir";
        case IntentType::ReadFile:
            return "read_file";
        case IntentType::SearchText:
            return "search_text";
        case IntentType::ConfigureProject:
            return "configure_project";
        case IntentType::BuildProject:
            return "build_project";
        case IntentType::TestProject:
            return "test_project";
    }
    return "unknown";
}

}  // namespace agent_tui
