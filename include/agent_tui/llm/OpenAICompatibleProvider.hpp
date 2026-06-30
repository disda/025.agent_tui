#pragma once

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <functional>
#include <fstream>
#include <map>
#include <regex>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <curl/curl.h>

#include "agent_tui/config/Config.hpp"
#include "agent_tui/llm/Provider.hpp"

namespace agent_tui {

namespace openai_compatible_detail {

inline std::string json_escape_string(const std::string& value) {
    std::ostringstream out;
    for (const char ch : value) {
        switch (ch) {
            case '\\': out << "\\\\"; break;
            case '"': out << "\\\""; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default: out << ch; break;
        }
    }
    return out.str();
}

inline int json_hex_value(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }
    if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    }
    return -1;
}

inline bool parse_json_hex4(const std::string& value, std::size_t offset, unsigned int& codepoint) {
    if (offset + 4 > value.size()) {
        return false;
    }
    unsigned int parsed = 0;
    for (std::size_t i = 0; i < 4; ++i) {
        const int hex = json_hex_value(value[offset + i]);
        if (hex < 0) {
            return false;
        }
        parsed = (parsed << 4) | static_cast<unsigned int>(hex);
    }
    codepoint = parsed;
    return true;
}

inline void append_utf8(std::ostringstream& out, unsigned int codepoint) {
    if (codepoint <= 0x7F) {
        out << static_cast<char>(codepoint);
        return;
    }
    if (codepoint <= 0x7FF) {
        out << static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F));
        out << static_cast<char>(0x80 | (codepoint & 0x3F));
        return;
    }
    if (codepoint <= 0xFFFF) {
        out << static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F));
        out << static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out << static_cast<char>(0x80 | (codepoint & 0x3F));
        return;
    }
    if (codepoint <= 0x10FFFF) {
        out << static_cast<char>(0xF0 | ((codepoint >> 18) & 0x07));
        out << static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        out << static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out << static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

inline std::string role_to_string(Role role) {
    switch (role) {
        case Role::System: return "system";
        case Role::User: return "user";
        case Role::Assistant: return "assistant";
        case Role::Tool: return "tool";
    }
    return "user";
}

inline std::string json_unescape_string(const std::string& value) {
    std::ostringstream out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        if (value[i] != '\\' || i + 1 >= value.size()) {
            out << value[i];
            continue;
        }
        const char next = value[++i];
        switch (next) {
            case 'n': out << '\n'; break;
            case 'r': out << '\r'; break;
            case 't': out << '\t'; break;
            case '"': out << '"'; break;
            case '\\': out << '\\'; break;
            case 'u': {
                unsigned int codepoint = 0;
                if (!parse_json_hex4(value, i + 1, codepoint)) {
                    out << 'u';
                    break;
                }
                i += 4;
                if (codepoint >= 0xD800 && codepoint <= 0xDBFF && i + 6 < value.size() &&
                    value[i + 1] == '\\' && value[i + 2] == 'u') {
                    unsigned int low = 0;
                    if (parse_json_hex4(value, i + 3, low) && low >= 0xDC00 && low <= 0xDFFF) {
                        codepoint = 0x10000 + (((codepoint - 0xD800) << 10) | (low - 0xDC00));
                        i += 6;
                    }
                }
                append_utf8(out, codepoint);
                break;
            }
            default: out << next; break;
        }
    }
    return out.str();
}

inline std::string extract_json_string_field(const std::string& body, const std::string& field) {
    const auto key = "\"" + field + "\"";
    auto pos = body.find(key);
    if (pos == std::string::npos) {
        return {};
    }
    pos = body.find(':', pos + key.size());
    if (pos == std::string::npos) {
        return {};
    }
    pos = body.find('"', pos + 1);
    if (pos == std::string::npos) {
        return {};
    }

    std::string value;
    bool escaped = false;
    for (std::size_t i = pos + 1; i < body.size(); ++i) {
        const char ch = body[i];
        if (escaped) {
            value.push_back('\\');
            value.push_back(ch);
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            return json_unescape_string(value);
        }
        value.push_back(ch);
    }
    return {};
}

inline JsonLike parse_flat_json_string_object(const std::string& body) {
    JsonLike result;
    std::size_t pos = 0;
    auto skip_ws = [&](std::size_t& index) {
        while (index < body.size() && std::isspace(static_cast<unsigned char>(body[index])) != 0) {
            ++index;
        }
    };
    auto parse_json_string = [&](std::size_t& index) {
        std::string value;
        bool escaped = false;
        if (index >= body.size() || body[index] != '"') {
            return value;
        }
        for (++index; index < body.size(); ++index) {
            const char ch = body[index];
            if (escaped) {
                value.push_back('\\');
                value.push_back(ch);
                escaped = false;
                continue;
            }
            if (ch == '\\') {
                escaped = true;
                continue;
            }
            if (ch == '"') {
                ++index;
                return json_unescape_string(value);
            }
            value.push_back(ch);
        }
        return json_unescape_string(value);
    };
    auto parse_compound = [&](std::size_t& index) {
        const char open = body[index];
        const char close = open == '{' ? '}' : ']';
        const auto start = index;
        int depth = 0;
        bool in_string = false;
        bool escaped = false;
        for (; index < body.size(); ++index) {
            const char ch = body[index];
            if (in_string) {
                if (escaped) {
                    escaped = false;
                } else if (ch == '\\') {
                    escaped = true;
                } else if (ch == '"') {
                    in_string = false;
                }
                continue;
            }
            if (ch == '"') {
                in_string = true;
                continue;
            }
            if (ch == open) {
                ++depth;
            } else if (ch == close) {
                --depth;
                if (depth == 0) {
                    ++index;
                    return body.substr(start, index - start);
                }
            }
        }
        return body.substr(start);
    };

    while (pos < body.size()) {
        skip_ws(pos);
        if (pos < body.size() && (body[pos] == '{' || body[pos] == ',')) {
            ++pos;
            continue;
        }
        pos = body.find('"', pos);
        if (pos == std::string::npos) {
            break;
        }
        auto key = parse_json_string(pos);
        skip_ws(pos);
        if (pos >= body.size() || body[pos] != ':') {
            break;
        }
        ++pos;
        skip_ws(pos);
        if (pos == std::string::npos) {
            break;
        }

        if (pos < body.size() && body[pos] == '"') {
            result[key] = parse_json_string(pos);
            continue;
        }
        if (pos < body.size() && (body[pos] == '{' || body[pos] == '[')) {
            result[key] = parse_compound(pos);
            continue;
        }

        const auto value_start = pos;
        while (pos < body.size() && body[pos] != ',' && body[pos] != '}') {
            ++pos;
        }
        auto value = body.substr(value_start, pos - value_start);
        value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
        }));
        value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
            return std::isspace(ch) == 0;
        }).base(), value.end());
        result[key] = value;
    }
    return result;
}

inline std::string flat_json_string_object(const JsonLike& object) {
    std::ostringstream out;
    out << "{";
    bool first = true;
    for (const auto& item : object) {
        if (!first) {
            out << ",";
        }
        first = false;
        out << "\"" << json_escape_string(item.first) << "\":\"" << json_escape_string(item.second) << "\"";
    }
    out << "}";
    return out.str();
}

inline std::string tool_calls_json(const std::vector<ToolCall>& calls) {
    std::ostringstream out;
    out << "[";
    bool first = true;
    for (const auto& call : calls) {
        if (!first) {
            out << ",";
        }
        first = false;
        out << "{";
        out << "\"id\":\"" << json_escape_string(call.id) << "\",";
        out << "\"type\":\"function\",";
        out << "\"function\":{";
        out << "\"name\":\"" << json_escape_string(call.name) << "\",";
        out << "\"arguments\":\"" << json_escape_string(flat_json_string_object(call.arguments)) << "\"";
        out << "}";
        out << "}";
    }
    out << "]";
    return out.str();
}

inline std::vector<ToolCall> extract_tool_calls(const std::string& body) {
    std::vector<ToolCall> calls;
    const std::regex pattern(
        R"REGEX("id"\s*:\s*"([^"]+)"[\s\S]*?"function"\s*:\s*\{[\s\S]*?"name"\s*:\s*"([^"]+)"[\s\S]*?"arguments"\s*:\s*"((?:\\.|[^"\\])*)")REGEX");
    auto begin = std::sregex_iterator(body.begin(), body.end(), pattern);
    const auto end = std::sregex_iterator();
    for (auto it = begin; it != end; ++it) {
        ToolCall call;
        call.id = (*it)[1].str();
        call.name = (*it)[2].str();
        call.arguments = parse_flat_json_string_object(json_unescape_string((*it)[3].str()));
        calls.push_back(std::move(call));
    }
    return calls;
}

inline std::string shell_quote(const std::filesystem::path& path) {
    auto value = path.generic_string();
    std::string quoted = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            quoted += "\\\"";
        } else {
            quoted += ch;
        }
    }
    quoted += "\"";
    return quoted;
}

inline std::string read_file(const std::filesystem::path& path) {
    std::ifstream input(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << input.rdbuf();
    return buffer.str();
}

inline bool write_file(const std::filesystem::path& path, const std::string& content) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return false;
    }
    output << content;
    return static_cast<bool>(output);
}

}  // namespace openai_compatible_detail

class OpenAICompatibleProvider final : public Provider {
public:
    explicit OpenAICompatibleProvider(Config config) : config_(std::move(config)) {}

    ProviderResponse chat(const std::vector<Message>& messages, const std::string& tools_schema_json = {}) override {
        const auto api_key = resolve_api_key(config_);
        if (api_key.empty()) {
            return ProviderResponse::error_response("missing API key: set api_key or api_key_env");
        }

        const auto api_base = config_.api_base.empty() ? std::string{"https://api.openai.com/v1"} : config_.api_base;
        const auto request_body = build_request_body(config_, messages, tools_schema_json);
        if (std::getenv("AGENT_TUI_DEBUG_REQUEST") != nullptr) {
            std::filesystem::create_directories("output");
            openai_compatible_detail::write_file(std::filesystem::path{"output"} / "last-openai-request.json", request_body);
        }

        return chat_with_libcurl(api_base, api_key, request_body);
    }

    ProviderResponse chat_stream(const std::vector<Message>& messages,
                                 const std::string& tools_schema_json,
                                 const std::function<void(const std::string&)>& on_delta) override {
        const auto api_key = resolve_api_key(config_);
        if (api_key.empty()) {
            return ProviderResponse::error_response("missing API key: set api_key or api_key_env");
        }

        const auto api_base = config_.api_base.empty() ? std::string{"https://api.openai.com/v1"} : config_.api_base;
        const auto request_body = build_request_body(config_, messages, tools_schema_json, true, true);
        return chat_stream_with_libcurl(api_base, api_key, request_body, on_delta);
    }

    static std::string build_request_body(const Config& config,
                                          const std::vector<Message>& messages,
                                          const std::string& tools_schema_json = {},
                                          bool include_tools = true,
                                          bool stream = false) {
        std::ostringstream out;
        out << "{";
        out << "\"model\":\"" << openai_compatible_detail::json_escape_string(config.model) << "\",";
        out << "\"messages\":[";
        bool first = true;
        for (const auto& message : messages) {
            if (!first) {
                out << ",";
            }
            first = false;
            out << "{";
            if (message.role == Role::Tool) {
                out << "\"role\":\"tool\",";
                out << "\"tool_call_id\":\"" << openai_compatible_detail::json_escape_string(message.tool_call_id) << "\",";
                out << "\"content\":\"" << openai_compatible_detail::json_escape_string(message.content) << "\"";
            } else if (message.role == Role::Assistant && !message.tool_calls.empty()) {
                out << "\"role\":\"assistant\",";
                out << "\"content\":\"" << openai_compatible_detail::json_escape_string(message.content) << "\",";
                out << "\"tool_calls\":" << openai_compatible_detail::tool_calls_json(message.tool_calls);
            } else {
                out << "\"role\":\"" << openai_compatible_detail::role_to_string(message.role) << "\",";
                out << "\"content\":\"" << openai_compatible_detail::json_escape_string(message.content) << "\"";
            }
            out << "}";
        }
        out << "],";
        if (include_tools && !tools_schema_json.empty()) {
            out << "\"tools\":" << tools_schema_json << ",";
            out << "\"tool_choice\":\"auto\",";
        }
        out << "\"stream\":" << (stream ? "true" : "false");
        out << "}";
        return out.str();
    }

    static std::string build_request_body(const Config& config, const std::vector<Message>& messages, bool include_tools, bool stream = false) {
        return build_request_body(config, messages, std::string{}, include_tools, stream);
    }

    static ProviderResponse parse_response_body(const std::string& body) {
        const auto tool_calls = openai_compatible_detail::extract_tool_calls(body);
        if (!tool_calls.empty()) {
            return ProviderResponse::tool_calls_response(tool_calls);
        }

        const auto error_message = openai_compatible_detail::extract_json_string_field(body, "message");
        if (body.find("\"error\"") != std::string::npos && !error_message.empty()) {
            return ProviderResponse::error_response(error_message);
        }

        const auto content = openai_compatible_detail::extract_json_string_field(body, "content");
        if (!content.empty()) {
            return ProviderResponse::text_response(content);
        }
        return ProviderResponse::error_response("failed to parse openai-compatible response");
    }

    static ProviderResponse parse_stream_chunks_for_test(const std::vector<std::string>& chunks) {
        return parse_stream_chunks(chunks, [](const std::string&) {});
    }

    static std::string resolve_api_key(const Config& config) {
        if (!config.api_key.empty()) {
            return config.api_key;
        }
        if (config.api_key_env.empty()) {
            return {};
        }
        const char* value = std::getenv(config.api_key_env.c_str());
        if (value == nullptr) {
            return {};
        }
        return value;
    }

private:
    struct StreamToolCallPart {
        std::string id;
        std::string name;
        std::string arguments;
    };

    struct StreamAccumulator {
        std::string text;
        std::map<int, StreamToolCallPart> tool_calls;
    };

    struct CurlStreamContext {
        StreamAccumulator accumulator;
        std::string raw;
        std::string pending_line;
        const std::function<void(const std::string&)>* on_delta = nullptr;
    };

    static std::string curl_error_message(CURLcode code, long status_code, const std::string& response_body) {
        std::ostringstream out;
        if (code != CURLE_OK) {
            out << "HTTP transport failed: " << curl_easy_strerror(code);
            return out.str();
        }
        out << "HTTP " << status_code;
        if (!response_body.empty()) {
            out << ": " << response_body.substr(0, 800);
        }
        return out.str();
    }

    static size_t curl_write_to_string(char* ptr, size_t size, size_t nmemb, void* userdata) {
        const auto bytes = size * nmemb;
        auto* output = static_cast<std::string*>(userdata);
        output->append(ptr, bytes);
        return bytes;
    }

    static void consume_stream_line(CurlStreamContext& context, std::string line) {
        context.raw += line;
        const auto chunk = normalize_stream_chunk(std::move(line));
        if (!chunk.empty()) {
            consume_stream_chunk(chunk, context.accumulator, *context.on_delta);
        }
    }

    static size_t curl_write_stream(char* ptr, size_t size, size_t nmemb, void* userdata) {
        const auto bytes = size * nmemb;
        auto* context = static_cast<CurlStreamContext*>(userdata);
        context->pending_line.append(ptr, bytes);
        std::size_t newline = 0;
        while ((newline = context->pending_line.find('\n')) != std::string::npos) {
            auto line = context->pending_line.substr(0, newline + 1);
            context->pending_line.erase(0, newline + 1);
            consume_stream_line(*context, std::move(line));
        }
        return bytes;
    }

    static curl_slist* append_header(curl_slist* headers, const std::string& header) {
        return curl_slist_append(headers, header.c_str());
    }

    ProviderResponse chat_with_libcurl(const std::string& api_base,
                                       const std::string& api_key,
                                       const std::string& request_body) const {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
        if (!curl) {
            return ProviderResponse::error_response("failed to initialize libcurl");
        }

        std::string response_body;
        const auto url = api_base + "/chat/completions";
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(request_body.size()));
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, static_cast<long>(config_.timeout_seconds));
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curl_write_to_string);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &response_body);

        curl_slist* headers = nullptr;
        headers = append_header(headers, "Content-Type: application/json");
        headers = append_header(headers, "Accept: application/json");
        headers = append_header(headers, "Authorization: Bearer " + api_key);
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);

        const CURLcode code = curl_easy_perform(curl.get());
        long status_code = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);
        if (headers != nullptr) {
            curl_slist_free_all(headers);
        }
        if (code != CURLE_OK || status_code < 200 || status_code >= 300) {
            return ProviderResponse::error_response(curl_error_message(code, status_code, response_body));
        }
        return parse_response_body(response_body);
    }

    ProviderResponse chat_stream_with_libcurl(const std::string& api_base,
                                              const std::string& api_key,
                                              const std::string& request_body,
                                              const std::function<void(const std::string&)>& on_delta) const {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> curl(curl_easy_init(), curl_easy_cleanup);
        if (!curl) {
            return ProviderResponse::error_response("failed to initialize libcurl");
        }

        CurlStreamContext context;
        context.on_delta = &on_delta;
        const auto url = api_base + "/chat/completions";
        curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POST, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDS, request_body.c_str());
        curl_easy_setopt(curl.get(), CURLOPT_POSTFIELDSIZE, static_cast<long>(request_body.size()));
        curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, static_cast<long>(config_.timeout_seconds));
        curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_LIMIT, 1L);
        curl_easy_setopt(curl.get(), CURLOPT_LOW_SPEED_TIME, static_cast<long>((std::max)(config_.timeout_seconds, 300)));
        curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, curl_write_stream);
        curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &context);

        curl_slist* headers = nullptr;
        headers = append_header(headers, "Content-Type: application/json");
        headers = append_header(headers, "Accept: text/event-stream");
        headers = append_header(headers, "Authorization: Bearer " + api_key);
        curl_easy_setopt(curl.get(), CURLOPT_HTTPHEADER, headers);

        const CURLcode code = curl_easy_perform(curl.get());
        if (!context.pending_line.empty()) {
            consume_stream_line(context, context.pending_line);
            context.pending_line.clear();
        }
        long status_code = 0;
        curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status_code);
        if (headers != nullptr) {
            curl_slist_free_all(headers);
        }
        if (code != CURLE_OK || status_code < 200 || status_code >= 300) {
            return ProviderResponse::error_response(curl_error_message(code, status_code, context.raw));
        }
        auto response = finalize_stream(context.accumulator);
        if (response.type != ProviderResponseType::Error) {
            return response;
        }
        return parse_response_body(context.raw);
    }

    static ProviderResponse parse_stream_chunks(const std::vector<std::string>& chunks,
                                                const std::function<void(const std::string&)>& on_delta) {
        StreamAccumulator accumulator;
        for (const auto& chunk : chunks) {
            const auto normalized = normalize_stream_chunk(chunk);
            if (!normalized.empty()) {
                consume_stream_chunk(normalized, accumulator, on_delta);
            }
        }
        return finalize_stream(accumulator);
    }

    static std::string normalize_stream_chunk(std::string line) {
        if (line.rfind("data:", 0) == 0) {
            line = line.substr(5);
        }
        while (!line.empty() && (line.front() == ' ' || line.front() == '\t')) {
            line.erase(line.begin());
        }
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) {
            line.pop_back();
        }
        if (line.empty() || line == "[DONE]") {
            return {};
        }
        return line;
    }

    static void consume_stream_chunk(const std::string& chunk,
                                     StreamAccumulator& accumulator,
                                     const std::function<void(const std::string&)>& on_delta) {
        const auto delta = openai_compatible_detail::extract_json_string_field(chunk, "content");
        if (!delta.empty()) {
            accumulator.text += delta;
            on_delta(delta);
        }

        std::size_t pos = 0;
        while ((pos = chunk.find("\"index\"", pos)) != std::string::npos) {
            auto colon = chunk.find(':', pos + 7);
            if (colon == std::string::npos) {
                break;
            }
            ++colon;
            while (colon < chunk.size() && std::isspace(static_cast<unsigned char>(chunk[colon])) != 0) {
                ++colon;
            }
            auto end = colon;
            while (end < chunk.size() && std::isdigit(static_cast<unsigned char>(chunk[end])) != 0) {
                ++end;
            }
            if (end == colon) {
                pos = end;
                continue;
            }

            const int index = std::stoi(chunk.substr(colon, end - colon));
            const auto next = chunk.find("\"index\"", end);
            const auto segment = chunk.substr(pos, next == std::string::npos ? std::string::npos : next - pos);

            auto& part = accumulator.tool_calls[index];
            const auto id = openai_compatible_detail::extract_json_string_field(segment, "id");
            const auto name = openai_compatible_detail::extract_json_string_field(segment, "name");
            const auto arguments = openai_compatible_detail::extract_json_string_field(segment, "arguments");
            if (!id.empty()) {
                part.id = id;
            }
            if (!name.empty()) {
                part.name = name;
            }
            if (!arguments.empty()) {
                part.arguments += arguments;
            }

            pos = next == std::string::npos ? chunk.size() : next;
        }
    }

    static ProviderResponse finalize_stream(const StreamAccumulator& accumulator) {
        if (!accumulator.tool_calls.empty()) {
            std::vector<ToolCall> calls;
            for (const auto& [index, part] : accumulator.tool_calls) {
                if (part.name.empty()) {
                    continue;
                }
                ToolCall call;
                call.id = part.id.empty() ? "stream_tool_call_" + std::to_string(index) : part.id;
                call.name = part.name;
                call.arguments = openai_compatible_detail::parse_flat_json_string_object(part.arguments);
                calls.push_back(std::move(call));
            }
            if (!calls.empty()) {
                return ProviderResponse::tool_calls_response(std::move(calls));
            }
        }
        if (!accumulator.text.empty()) {
            return ProviderResponse::text_response(accumulator.text);
        }
        return ProviderResponse::error_response("failed to parse openai-compatible streaming response");
    }

    Config config_;
};

}  // namespace agent_tui
