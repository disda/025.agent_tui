#include "agent_tui/intent/IntentClassifier.hpp"

#include <cassert>
#include <string>

using namespace agent_tui;

namespace {

std::string bytes(std::initializer_list<unsigned char> values) {
    std::string out;
    out.reserve(values.size());
    for (const auto value : values) {
        out.push_back(static_cast<char>(value));
    }
    return out;
}

std::string zh_read_readme() {
    return bytes({0xE8,0xAF,0xBB,0xE5,0x8F,0x96,' ','R','E','A','D','M','E','.','m','d'});
}

std::string zh_search_runner() {
    return bytes({0xE6,0x90,0x9C,0xE7,0xB4,0xA2,' ','A','g','e','n','t','R','u','n','n','e','r'});
}

std::string zh_build() {
    return bytes({0xE7,0xBC,0x96,0xE8,0xAF,0x91});
}

std::string zh_test() {
    return bytes({0xE8,0xBF,0x90,0xE8,0xA1,0x8C,0xE6,0xB5,0x8B,0xE8,0xAF,0x95});
}

}  // namespace

void test_list_dir_intent() {
    auto intent = IntentClassifier::classify("ls");
    assert(intent.type == IntentType::ListDir);
    assert(intent.argument == ".");

    intent = IntentClassifier::classify("list docs");
    assert(intent.type == IntentType::ListDir);
    assert(intent.argument == "docs");
}

void test_read_file_intent() {
    auto intent = IntentClassifier::classify("read README.md");
    assert(intent.type == IntentType::ReadFile);
    assert(intent.argument == "README.md");

    intent = IntentClassifier::classify(zh_read_readme());
    assert(intent.type == IntentType::ReadFile);
    assert(intent.argument == "README.md");
}

void test_search_text_intent() {
    auto intent = IntentClassifier::classify("search AgentRunner");
    assert(intent.type == IntentType::SearchText);
    assert(intent.argument == "AgentRunner");

    intent = IntentClassifier::classify(zh_search_runner());
    assert(intent.type == IntentType::SearchText);
    assert(intent.argument == "AgentRunner");
}

void test_build_and_test_intents() {
    auto intent = IntentClassifier::classify("build");
    assert(intent.type == IntentType::BuildProject);

    intent = IntentClassifier::classify(zh_build());
    assert(intent.type == IntentType::BuildProject);

    intent = IntentClassifier::classify("ctest");
    assert(intent.type == IntentType::TestProject);

    intent = IntentClassifier::classify(zh_test());
    assert(intent.type == IntentType::TestProject);
}

void test_unknown_intent() {
    auto intent = IntentClassifier::classify("tell me a joke");
    assert(intent.type == IntentType::Unknown);
}

int main() {
    test_list_dir_intent();
    test_read_file_intent();
    test_search_text_intent();
    test_build_and_test_intents();
    test_unknown_intent();
    return 0;
}
