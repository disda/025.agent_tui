#include "agent_tui/tui/TuiApp.hpp"

#include <cassert>
#include <sstream>
#include <string>

using namespace agent_tui;

void test_model_command_sets_model() {
    TuiApp app;
    assert(app.handle_command("/model gpt-test"));
    assert(app.config().model == "gpt-test");
}

void test_api_commands_set_runtime_config_without_key_value() {
    TuiApp app;
    assert(app.handle_command("/api provider openai"));
    assert(app.handle_command("/api base https://example.com/v1"));
    assert(app.handle_command("/api key-env OPENAI_API_KEY"));
    assert(app.handle_command("/api timeout 45"));
    assert(app.handle_command("/api max-loops 9"));

    assert(app.config().provider == "openai");
    assert(app.config().api_base == "https://example.com/v1");
    assert(app.config().api_key_env == "OPENAI_API_KEY");
    assert(app.config().timeout_seconds == 45);
    assert(app.config().max_loops == 9);
}

void test_config_command_paths_and_show() {
    TuiApp app;
    assert(app.handle_command("/config show"));
    assert(app.handle_command("/config paths"));
}

void test_interrupt_command_sets_flag() {
    TuiApp app;
    assert(!app.interrupted());
    assert(app.handle_command("/interrupt"));
    assert(app.interrupted());
}

void test_clear_command_resets_history_and_interrupt() {
    TuiApp app;
    app.handle_command("/interrupt");
    app.handle_command("/status");
    assert(app.interrupted());
    assert(!app.history().empty());

    assert(app.handle_command("/clear"));
    assert(!app.interrupted());
    assert(app.history().size() == 1);  // /clear records "session cleared"
}

void test_exit_command_stops_app() {
    TuiApp app;
    assert(app.running());
    assert(app.handle_command("/exit"));
    assert(!app.running());
}

void test_run_accepts_scripted_input_and_uses_mock_provider() {
    std::istringstream input("hello provider\n/model scripted\n/api provider mock\n/exit\n");
    std::ostringstream output;

    TuiApp app;
    app.set_streams(input, output);
    const auto code = app.run();

    assert(code == 0);
    assert(app.config().model == "scripted");
    assert(app.config().provider == "mock");
    assert(output.str().find("agent_tui") != std::string::npos);
    assert(output.str().find("mock assistant: hello provider") != std::string::npos);
}

int main() {
    test_model_command_sets_model();
    test_api_commands_set_runtime_config_without_key_value();
    test_config_command_paths_and_show();
    test_interrupt_command_sets_flag();
    test_clear_command_resets_history_and_interrupt();
    test_exit_command_stops_app();
    test_run_accepts_scripted_input_and_uses_mock_provider();
    return 0;
}
