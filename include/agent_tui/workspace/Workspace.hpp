#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>

namespace agent_tui {

class Workspace {
public:
    explicit Workspace(std::filesystem::path root)
        : root_(std::filesystem::weakly_canonical(std::filesystem::absolute(std::move(root)))) {
        if (!std::filesystem::exists(root_)) {
            throw std::runtime_error("workspace root does not exist: " + root_.string());
        }
        if (!std::filesystem::is_directory(root_)) {
            throw std::runtime_error("workspace root is not a directory: " + root_.string());
        }
    }

    const std::filesystem::path& root() const { return root_; }

    std::filesystem::path resolve(const std::string& user_path) const {
        const auto raw = user_path.empty() ? std::filesystem::path{"."} : std::filesystem::path{user_path};
        const auto candidate = raw.is_absolute() ? raw : root_ / raw;
        const auto normalized = std::filesystem::weakly_canonical(std::filesystem::absolute(candidate));
        if (!is_inside(normalized)) {
            throw std::runtime_error("path escapes workspace: " + user_path);
        }
        return normalized;
    }

    std::string display_path(const std::filesystem::path& path) const {
        const auto normalized = std::filesystem::weakly_canonical(std::filesystem::absolute(path));
        if (normalized == root_) {
            return ".";
        }
        auto relative = normalized.lexically_relative(root_);
        if (relative.empty()) {
            return normalized.generic_string();
        }
        return relative.generic_string();
    }

private:
    bool is_inside(const std::filesystem::path& path) const {
        if (path == root_) {
            return true;
        }
        auto relative = path.lexically_relative(root_);
        if (relative.empty()) {
            return false;
        }
        for (const auto& part : relative) {
            if (part == "..") {
                return false;
            }
        }
        return true;
    }

    std::filesystem::path root_;
};

}  // namespace agent_tui
