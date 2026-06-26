# 验证说明：Config + Provider + Terminal Chat

日期：2026-06-26

## 1. 验证目标

本次验证目标是接入类似 `.codex` 的本地配置目录、Provider 工厂和终端对话链路。

目标是让用户可以：

```text
配置 provider / model / api_base / api_key_env
初始化用户配置目录
在终端中输入普通文本并获得 Provider 回复
```

## 2. 新增内容

新增：

```text
include/agent_tui/config/Config.hpp
include/agent_tui/config/ConfigLoader.hpp
include/agent_tui/llm/EchoProvider.hpp
include/agent_tui/llm/ProviderFactory.hpp
tests/test_config_loader.cpp
```

更新：

```text
include/agent_tui/tui/TuiConfig.hpp
include/agent_tui/tui/TuiApp.hpp
CMakeLists.txt
```

## 3. 配置目录

用户级配置：

```text
~/.agent_tui/config.toml
```

项目级配置：

```text
./.agent_tui/config.toml
```

加载优先级：

```text
默认值 < 用户级配置 < 项目级配置 < TUI 运行时命令
```

配置示例：

```toml
provider = "mock"
model = "mock-model"
api_base = ""
api_key_env = "OPENAI_API_KEY"
timeout_seconds = 60
max_loops = 8
```

## 4. API Key 安全策略

配置文件只保存 API Key 的环境变量名：

```toml
api_key_env = "OPENAI_API_KEY"
```

程序只展示：

```text
api_key_env: OPENAI_API_KEY
api_key_status: OPENAI_API_KEY=<set|missing>
```

不会打印真实 API Key。

## 5. ProviderFactory

当前 Provider 策略：

```text
mock / echo -> EchoProvider
openai-compatible / codingplan / others -> PlaceholderProvider
```

`EchoProvider` 用于本地终端对话验证：

```text
user: hello
assistant: mock assistant: hello
```

真实 HTTP Provider 后续单独实现。

## 6. TUI 新增命令

新增 `/config`：

```text
/config show
/config paths
/config init user
/config reload
```

已有 `/api` 仍用于运行时修改配置：

```text
/api provider mock
/api base https://api.openai.com/v1
/api key-env OPENAI_API_KEY
/api timeout 60
/api max-loops 8
```

## 7. Terminal Chat 链路

普通输入现在会走 Provider：

```text
用户输入
  ↓
SessionHistory 记录 user_input
  ↓
ProviderFactory 根据 config 创建 Provider
  ↓
Provider.chat(messages)
  ↓
展示 assistant 回复或 provider error
  ↓
SessionHistory 记录 assistant_message / error
```

## 8. 测试覆盖

`tests/test_config_loader.cpp` 覆盖：

- 项目级配置覆盖用户级配置。
- API Key 不暴露。
- 示例配置创建。
- mock provider chat。
- 未实现 provider 返回 placeholder error。

`tests/test_tui_app.cpp` 覆盖：

- `/config show`。
- `/config paths`。
- 普通输入能通过 mock provider 得到 assistant 回复。

## 9. 验证命令

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 10. 当前限制

第一版限制：

- `openai-compatible` 和 `codingplan` 只是 placeholder。
- 暂未做真实 HTTP 请求。
- 暂未做 tool schema 到 Provider 的转换。
- 暂未做 streaming。
- 暂未做配置落盘保存运行时修改。

## 11. 下一步

下一步建议：

```text
feat: add openai compatible provider skeleton
```

然后：

```text
feat: add codingplan provider skeleton
```
