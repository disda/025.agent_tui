# 验证说明：OpenAI-compatible Provider

日期：2026-06-26

## 1. 验证目标

本次验证目标是直接接入 OpenAI-compatible Provider，让用户可以通过 TUI 使用真实 OpenAI-compatible API 做文本对话验证。

## 2. 新增内容

新增：

```text
include/agent_tui/llm/OpenAICompatibleProvider.hpp
tests/test_openai_compatible_provider.cpp
```

更新：

```text
include/agent_tui/llm/ProviderFactory.hpp
CMakeLists.txt
```

## 3. 当前能力

Provider 名称：

```text
openai-compatible
openai
```

支持：

- 从 `api_key_env` 读取 API Key。
- 使用 `api_base + /chat/completions` 发送请求。
- 构造 OpenAI-compatible chat request body。
- 解析普通文本 response。
- 解析 provider error response。

## 4. 验证方式

PowerShell：

```powershell
$env:OPENAI_API_KEY="你的 key"
.\build\Debug\agent_tui.exe
```

TUI 输入：

```text
/api provider openai-compatible
/api base https://api.openai.com/v1
/api key-env OPENAI_API_KEY
/model gpt-4.1
你好，回复一句话
```

也可以写入配置：

```text
~/.agent_tui/config.toml
```

```toml
provider = "openai-compatible"
model = "gpt-4.1"
api_base = "https://api.openai.com/v1"
api_key_env = "OPENAI_API_KEY"
timeout_seconds = 60
max_loops = 8
```

然后启动后执行：

```text
/config reload
你好，回复一句话
```

## 5. 测试覆盖

`tests/test_openai_compatible_provider.cpp` 覆盖：

- request body 构造包含 model 和 messages。
- response body 能解析 assistant text。
- error body 能解析错误消息。
- ProviderFactory 能创建 OpenAI-compatible provider。

## 6. 当前限制

第一版限制：

- 使用系统 `curl` 命令作为传输层。
- 暂不支持 streaming。
- 暂不支持 tool_calls 解析。
- 暂不支持 tools schema 转换。
- 暂不支持复杂 retry。

## 7. 下一步

下一步建议：

```text
feat: add openai compatible tool call parsing
```

或者开始：

```text
feat: add codingplan provider skeleton
```
