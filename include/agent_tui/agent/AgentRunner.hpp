#pragma once

#include <string>
#include <vector>

#include "agent_tui/agent/AgentResult.hpp"
#include "agent_tui/llm/Provider.hpp"
#include "agent_tui/tools/ToolRegistry.hpp"

namespace agent_tui {

class AgentRunner {
public:
    AgentRunner(Provider& provider, ToolRegistry& tools, int max_loops = 8)
        : provider_(provider), tools_(tools), max_loops_(max_loops) {}

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

                auto result = tool->run(call.arguments);
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
    int max_loops_ = 8;
    std::vector<Message> last_messages_;
};

}  // namespace agent_tui
