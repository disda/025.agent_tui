#include "agent_tui/llm/OpenAICompatibleProvider.hpp"
#include "agent_tui/llm/ProviderFactory.hpp"

#include <cassert>
#include <string>

using namespace agent_tui;

void test_build_request_body_contains_model_and_messages() {
    Config config;
    config.model = "gpt-test";

    const auto body = OpenAICompatibleProvider::build_request_body(config, {
        Message{Role::System, "You are terse.", {}},
        Message{Role::User, "hello world", {}},
    });

    assert(body.find("\"model\":\"gpt-test\"") != std::string::npos);
    assert(body.find("\"role\":\"system\"") != std::string::npos);
    assert(body.find("\"role\":\"user\"") != std::string::npos);
    assert(body.find("hello world") != std::string::npos);
    assert(body.find("\"stream\":false") != std::string::npos);
}

void test_parse_text_response() {
    const std::string body = R"({
        "id": "chatcmpl-test",
        "choices": [
            {"message": {"role": "assistant", "content": "hello from api"}}
        ]
    })";

    const auto response = OpenAICompatibleProvider::parse_response_body(body);
    assert(response.type == ProviderResponseType::Text);
    assert(response.text == "hello from api");
}

void test_parse_error_response() {
    const std::string body = R"({
        "error": {"message": "provider rejected request", "type": "invalid_request_error"}
    })";

    const auto response = OpenAICompatibleProvider::parse_response_body(body);
    assert(response.type == ProviderResponseType::Error);
    assert(response.error == "provider rejected request");
}

void test_provider_factory_creates_openai_compatible() {
    Config config;
    config.provider = "openai-compatible";
    config.api_key_env = "AGENT_TUI_TEST_MISSING_ENV";

    auto provider = ProviderFactory::create(config);
    const auto response = provider->chat({Message{Role::User, "hi", {}}});
    assert(response.type == ProviderResponseType::Error);
    assert(response.error.find("missing API key env") != std::string::npos);
}

int main() {
    test_build_request_body_contains_model_and_messages();
    test_parse_text_response();
    test_parse_error_response();
    test_provider_factory_creates_openai_compatible();
    return 0;
}
