#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "agent_tui/permissions/ApprovalService.hpp"

namespace agent_tui {

class MockApprovalService final : public ApprovalService {
public:
    explicit MockApprovalService(std::vector<ApprovalDecision> scripted_decisions)
        : scripted_decisions_(std::move(scripted_decisions)) {}

    ApprovalDecision request(const ToolCall& call, const Tool& tool) override {
        observed_calls_.push_back(call);
        observed_tool_names_.push_back(tool.name());
        if (index_ >= scripted_decisions_.size()) {
            return ApprovalDecision::deny("MockApprovalService has no more scripted decisions");
        }
        return scripted_decisions_[index_++];
    }

    std::size_t request_count() const { return index_; }
    const std::vector<ToolCall>& observed_calls() const { return observed_calls_; }
    const std::vector<std::string>& observed_tool_names() const { return observed_tool_names_; }

private:
    std::vector<ApprovalDecision> scripted_decisions_;
    std::vector<ToolCall> observed_calls_;
    std::vector<std::string> observed_tool_names_;
    std::size_t index_ = 0;
};

}  // namespace agent_tui
