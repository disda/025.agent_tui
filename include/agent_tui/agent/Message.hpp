#pragma once

#include <string>

namespace agent_tui {

enum class Role {
    System,
    User,
    Assistant,
    Tool,
};

struct Message {
    Role role = Role::User;
    std::string content;
    std::string tool_call_id;
};

}  // namespace agent_tui
