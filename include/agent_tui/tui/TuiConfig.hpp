#pragma once

#include <sstream>
#include <string>

namespace agent_tui {

struct TuiConfig {
    std::string provider = "mock";
    std::string model = "mock-model";
    std::string api_base;
    std::string api_key_env;
    int timeout_seconds = 60;
    int max_loops = 8;

    std::string summary() const {
        std::ostringstream out;
        out << "provider: " << provider << '\n';
        out << "model: " << model << '\n';
        out << "api_base: " << (api_base.empty() ? "<not set>" : api_base) << '\n';
        out << "api_key_env: " << (api_key_env.empty() ? "<not set>" : api_key_env) << '\n';
        out << "timeout_seconds: " << timeout_seconds << '\n';
        out << "max_loops: " << max_loops << '\n';
        return out.str();
    }
};

}  // namespace agent_tui
