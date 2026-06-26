#pragma once

#include <cstddef>
#include <vector>

#include "agent_tui/session/SessionEvent.hpp"

namespace agent_tui {

class SessionHistory {
public:
    void add(SessionEvent event) {
        events_.push_back(std::move(event));
    }

    const std::vector<SessionEvent>& events() const {
        return events_;
    }

    void clear() {
        events_.clear();
    }

    bool empty() const {
        return events_.empty();
    }

    std::size_t size() const {
        return events_.size();
    }

private:
    std::vector<SessionEvent> events_;
};

}  // namespace agent_tui
