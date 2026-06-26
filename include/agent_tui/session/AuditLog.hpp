#pragma once

#include <filesystem>
#include <fstream>
#include <utility>

#include "agent_tui/session/SessionHistory.hpp"

namespace agent_tui {

class AuditLog {
public:
    explicit AuditLog(std::filesystem::path path) : path_(std::move(path)) {}

    const std::filesystem::path& path() const {
        return path_;
    }

    bool append(const SessionEvent& event) const {
        const auto parent = path_.parent_path();
        if (!parent.empty()) {
            std::filesystem::create_directories(parent);
        }

        std::ofstream output(path_, std::ios::app);
        if (!output) {
            return false;
        }
        output << event.to_json_line() << '\n';
        return static_cast<bool>(output);
    }

    bool append_all(const SessionHistory& history) const {
        for (const auto& event : history.events()) {
            if (!append(event)) {
                return false;
            }
        }
        return true;
    }

private:
    std::filesystem::path path_;
};

}  // namespace agent_tui
