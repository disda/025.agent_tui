#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "agent_tui/tools/Tool.hpp"

namespace agent_tui {

class ToolRegistry {
public:
    void register_tool(std::unique_ptr<Tool> tool) {
        auto key = tool->name();
        tools_[key] = std::move(tool);
    }

    Tool* find(const std::string& name) const {
        auto it = tools_.find(name);
        if (it == tools_.end()) {
            return nullptr;
        }
        return it->second.get();
    }

    std::vector<std::string> names() const {
        std::vector<std::string> result;
        result.reserve(tools_.size());
        for (const auto& [name, _] : tools_) {
            result.push_back(name);
        }
        return result;
    }

private:
    std::unordered_map<std::string, std::unique_ptr<Tool>> tools_;
};

}  // namespace agent_tui
