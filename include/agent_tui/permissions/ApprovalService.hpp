#pragma once

#include "agent_tui/agent/ToolCall.hpp"
#include "agent_tui/permissions/Approval.hpp"
#include "agent_tui/tools/Tool.hpp"

namespace agent_tui {

class ApprovalService {
public:
    virtual ~ApprovalService() = default;

    virtual ApprovalDecision request(const ToolCall& call, const Tool& tool) = 0;
};

}  // namespace agent_tui
