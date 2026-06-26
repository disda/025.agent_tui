#pragma once

#include <string>
#include <vector>

#include "agent_tui/agent/AgentResult.hpp"
#include "agent_tui/llm/Provider.hpp"
#include "agent_tui/permissions/ApprovalService.hpp"
#include "agent_tui/tools/ToolRegistry.hpp"

namespace agent_tui {

class AgentRunner {
public:
    AgentRunner(Provider& provider, ToolRegistry& tools, int max_loops = 8)
        : provider_(provider), tools_(tools), max_loops_(max_loops) {}

    AgentRunner(Provider& provider, ToolRegistry& tools, ApprovalService& approval_service, int max_loops = 8)
        : provider_(provider), tools_(tools), approval_service_(&approval_service), max_loops_(max_loops) {}

    AgentResult run(std::vector<Message> messages) {
        for (int step = 0; step < max_loops_; ++step) {
            auto response = provider_.chat(messages);

            if (response.type == ProviderResponseType::Text) {
                messages.push_back(Message{Role::Assistant, response.text, {}});
                last_messages_ = messages;
                return AgentResult::done(response.text);
            }

            if (response.type == ProviderResponseType::Error) {
                last_messages_ = messages;
                return AgentResult::failed(response.error);
            }

            for (const auto& call : response.tool_calls) {
                if (call.name == "Done") {
                    auto it = call.arguments.find("final_answer");
                    const auto answer = it == call.arguments.end() ? std::string{"Done"} : it->second;
                    messages.push_back(Message{Role::Assistant, answer, {}});
                    last_messages_ = messages;
                    return AgentResult::done(answer);
                }

                auto* tool = tools_.find(call.name);
                if (tool == nullptr) {
                    messages.push_back(Message{Role::Tool, "Tool not found: " + call.name, call.id});
                    continue;
                }

                JsonLike arguments = call.arguments;
                if (tool->permission_mode() == PermissionMode::Confirm) {
                    if (approval_service_ == nullptr) {
                        messages.push_back(Message{Role::Tool, "Permission required but no approval service configured.", call.id});
                        continue;
                    }

                    auto decision = approval_service_->request(call, *tool);
                    if (decision.type == ApprovalType::Deny) {
                        auto message = decision.feedback.empty() ? std::string{"User denied permission."}
                                                                : std::string{"User denied permission: "} + decision.feedback;
                        messages.push_back(Message{Role::Tool, message, call.id});
                        continue;
                    }
                    if (decision.type == ApprovalType::Feedback) {
                        messages.push_back(Message{Role::Tool, "User feedback: " + decision.feedback, call.id});
                        continue;
                    }
                    if (decision.type == ApprovalType::Edit) {
                        arguments = decision.edited_arguments;
                    }
                }

                auto result = tool->run(arguments);
                messages.push_back(Message{
                    Role::Tool,
                    result.ok ? result.output : result.error,
                    call.id,
                });
            }
        }

        last_messages_ = messages;
        return AgentResult::failed("Max loop count exceeded.");
    }

    const std::vector<Message>& last_messages() const { return last_messages_; }

private:
    Provider& provider_;
    ToolRegistry& tools_;
    ApprovalService* approval_service_ = nullptr;
    int max_loops_ = 8;
    std::vector<Message> last_messages_;
};

}  // namespace agent_tui
