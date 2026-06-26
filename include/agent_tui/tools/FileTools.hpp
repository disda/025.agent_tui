#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include "agent_tui/tools/Tool.hpp"
#include "agent_tui/workspace/Workspace.hpp"

namespace agent_tui {

namespace file_tools_detail {

inline std::string get_arg(const JsonLike& arguments, const std::string& key, const std::string& fallback = {}) {
    const auto it = arguments.find(key);
    return it == arguments.end() ? fallback : it->second;
}

inline std::size_t get_size_arg(const JsonLike& arguments, const std::string& key, std::size_t fallback) {
    const auto it = arguments.find(key);
    if (it == arguments.end() || it->second.empty()) {
        return fallback;
    }
    try {
        return static_cast<std::size_t>(std::stoull(it->second));
    } catch (...) {
        return fallback;
    }
}

inline bool should_skip_dir(const std::filesystem::path& path) {
    const auto name = path.filename().string();
    return name == ".git" || name == "build" || name.rfind("cmake-build-", 0) == 0;
}

inline std::string glob_to_regex(const std::string& pattern) {
    std::string output = "^";
    for (const char c : pattern) {
        switch (c) {
            case '*':
                output += ".*";
                break;
            case '?':
                output += ".";
                break;
            case '.': case '+': case '(': case ')': case '^': case '$':
            case '|': case '{': case '}': case '[': case ']': case '\\':
                output += '\\';
                output += c;
                break;
            default:
                output += c;
                break;
        }
    }
    output += "$";
    return output;
}

inline std::string truncate(std::string value, std::size_t max_bytes) {
    if (value.size() <= max_bytes) {
        return value;
    }
    value.resize(max_bytes);
    value += "\n...[truncated]";
    return value;
}

}  // namespace file_tools_detail

class ListDirTool final : public Tool {
public:
    explicit ListDirTool(const Workspace& workspace) : workspace_(workspace) {}

    std::string name() const override { return "list_dir"; }
    std::string description() const override { return "List files and directories under a workspace path."; }
    PermissionMode permission_mode() const override { return PermissionMode::Auto; }

    ToolResult run(const JsonLike& arguments) override {
        try {
            const auto path = workspace_.resolve(file_tools_detail::get_arg(arguments, "path", "."));
            if (!std::filesystem::is_directory(path)) {
                return ToolResult::failure("not a directory: " + workspace_.display_path(path));
            }

            std::vector<std::string> entries;
            for (const auto& entry : std::filesystem::directory_iterator(path)) {
                auto label = entry.path().filename().generic_string();
                if (entry.is_directory()) {
                    label += "/";
                }
                entries.push_back(label);
            }
            std::sort(entries.begin(), entries.end());

            std::ostringstream out;
            for (const auto& entry : entries) {
                out << entry << '\n';
            }
            return ToolResult::success(out.str());
        } catch (const std::exception& error) {
            return ToolResult::failure(error.what());
        }
    }

private:
    const Workspace& workspace_;
};

class ReadFileTool final : public Tool {
public:
    explicit ReadFileTool(const Workspace& workspace) : workspace_(workspace) {}

    std::string name() const override { return "read_file"; }
    std::string description() const override { return "Read a UTF-8 text file from the workspace."; }
    PermissionMode permission_mode() const override { return PermissionMode::Auto; }

    ToolResult run(const JsonLike& arguments) override {
        try {
            const auto requested_path = file_tools_detail::get_arg(arguments, "path");
            if (requested_path.empty()) {
                return ToolResult::failure("missing required argument: path");
            }
            const auto path = workspace_.resolve(requested_path);
            if (!std::filesystem::is_regular_file(path)) {
                return ToolResult::failure("not a regular file: " + workspace_.display_path(path));
            }

            const auto max_bytes = file_tools_detail::get_size_arg(arguments, "max_bytes", 64 * 1024);
            std::ifstream input(path, std::ios::binary);
            if (!input) {
                return ToolResult::failure("failed to open file: " + workspace_.display_path(path));
            }
            std::ostringstream buffer;
            buffer << input.rdbuf();
            return ToolResult::success(file_tools_detail::truncate(buffer.str(), max_bytes));
        } catch (const std::exception& error) {
            return ToolResult::failure(error.what());
        }
    }

private:
    const Workspace& workspace_;
};

class GlobFilesTool final : public Tool {
public:
    explicit GlobFilesTool(const Workspace& workspace) : workspace_(workspace) {}

    std::string name() const override { return "glob_files"; }
    std::string description() const override { return "Find workspace files matching a glob pattern."; }
    PermissionMode permission_mode() const override { return PermissionMode::Auto; }

    ToolResult run(const JsonLike& arguments) override {
        try {
            const auto pattern = file_tools_detail::get_arg(arguments, "pattern");
            if (pattern.empty()) {
                return ToolResult::failure("missing required argument: pattern");
            }
            const auto start = workspace_.resolve(file_tools_detail::get_arg(arguments, "path", "."));
            const auto max_matches = file_tools_detail::get_size_arg(arguments, "max_matches", 100);
            const std::regex matcher(file_tools_detail::glob_to_regex(pattern));
            const bool match_full_relative_path = pattern.find('/') != std::string::npos;

            std::vector<std::string> matches;
            std::filesystem::recursive_directory_iterator it(start);
            const std::filesystem::recursive_directory_iterator end;
            for (; it != end && matches.size() < max_matches; ++it) {
                if (it->is_directory() && file_tools_detail::should_skip_dir(it->path())) {
                    it.disable_recursion_pending();
                    continue;
                }
                if (!it->is_regular_file()) {
                    continue;
                }
                const auto relative = workspace_.display_path(it->path());
                const auto candidate = match_full_relative_path ? relative : it->path().filename().generic_string();
                if (std::regex_match(candidate, matcher)) {
                    matches.push_back(relative);
                }
            }
            std::sort(matches.begin(), matches.end());

            std::ostringstream out;
            for (const auto& match : matches) {
                out << match << '\n';
            }
            return ToolResult::success(out.str());
        } catch (const std::exception& error) {
            return ToolResult::failure(error.what());
        }
    }

private:
    const Workspace& workspace_;
};

class SearchTextTool final : public Tool {
public:
    explicit SearchTextTool(const Workspace& workspace) : workspace_(workspace) {}

    std::string name() const override { return "search_text"; }
    std::string description() const override { return "Search text in workspace files."; }
    PermissionMode permission_mode() const override { return PermissionMode::Auto; }

    ToolResult run(const JsonLike& arguments) override {
        try {
            const auto query = file_tools_detail::get_arg(arguments, "query");
            if (query.empty()) {
                return ToolResult::failure("missing required argument: query");
            }
            const auto start = workspace_.resolve(file_tools_detail::get_arg(arguments, "path", "."));
            const auto max_matches = file_tools_detail::get_size_arg(arguments, "max_matches", 50);
            const auto max_file_bytes = file_tools_detail::get_size_arg(arguments, "max_file_bytes", 1024 * 1024);

            std::vector<std::string> matches;
            std::filesystem::recursive_directory_iterator it(start);
            const std::filesystem::recursive_directory_iterator end;
            for (; it != end && matches.size() < max_matches; ++it) {
                if (it->is_directory() && file_tools_detail::should_skip_dir(it->path())) {
                    it.disable_recursion_pending();
                    continue;
                }
                if (!it->is_regular_file()) {
                    continue;
                }
                if (std::filesystem::file_size(it->path()) > max_file_bytes) {
                    continue;
                }

                std::ifstream input(it->path());
                if (!input) {
                    continue;
                }
                std::string line;
                std::size_t line_no = 0;
                while (std::getline(input, line) && matches.size() < max_matches) {
                    ++line_no;
                    if (line.find(query) != std::string::npos) {
                        std::ostringstream item;
                        item << workspace_.display_path(it->path()) << ':' << line_no << ':' << line;
                        matches.push_back(item.str());
                    }
                }
            }

            std::ostringstream out;
            for (const auto& match : matches) {
                out << match << '\n';
            }
            return ToolResult::success(out.str());
        } catch (const std::exception& error) {
            return ToolResult::failure(error.what());
        }
    }

private:
    const Workspace& workspace_;
};

}  // namespace agent_tui
