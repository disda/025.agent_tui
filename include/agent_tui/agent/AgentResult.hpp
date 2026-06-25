#pragma once

#include <string>
#include <utility>

namespace agent_tui {

enum class AgentResultStatus {
    Done,
    Failed,
};

struct AgentResult {
    AgentResultStatus status = AgentResultStatus::Done;
    std::string output;
    std::string error;

    static AgentResult done(std::string text) {
        AgentResult result;
        result.status = AgentResultStatus::Done;
        result.output = std::move(text);
        return result;
    }

    static AgentResult failed(std::string message) {
        AgentResult result;
        result.status = AgentResultStatus::Failed;
        result.error = std::move(message);
        return result;
    }

    bool ok() const { return status == AgentResultStatus::Done; }
};

}  // namespace agent_tui
