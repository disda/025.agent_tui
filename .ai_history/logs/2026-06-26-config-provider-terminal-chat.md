# AI 协作记录：Config + Provider + Terminal Chat

日期：2026-06-26

## 背景

用户希望实现类似 `.codex` 的本地配置目录，支持用户配置 API Key，并把 Provider 配置和终端对话先接起来。

## 本轮技术文档

新增：

```text
docs/19-config-provider-terminal-chat-design.md
```

文档明确：

- 用户级配置目录：`~/.agent_tui/config.toml`。
- 项目级配置目录：`./.agent_tui/config.toml`。
- 加载优先级：默认值 < 用户级配置 < 项目级配置 < TUI 运行时命令。
- API Key 只保存环境变量名，不保存真实 key。
- ProviderFactory 先支持 mock / echo，其他 provider 先返回 placeholder error。

## 本轮实现

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
README.md
TODO.md
docs/20-config-provider-terminal-chat-verification.md
```

## 当前新增能力

配置：

- `~/.agent_tui/config.toml`
- `./.agent_tui/config.toml`
- 项目级覆盖用户级
- 示例配置生成
- 脱敏 summary

TUI 命令：

```text
/config show
/config paths
/config init user
/config reload
```

Provider：

- `mock` / `echo` -> `EchoProvider`
- 其他 provider -> `PlaceholderProvider`

终端对话：

```text
user: hello
assistant: mock assistant: hello
```

## 测试覆盖

新增测试：

```text
test_project_config_overrides_user_config
test_api_key_is_not_exposed
test_write_example_config
test_provider_factory_mock_chat
test_provider_factory_placeholder_for_unknown_provider
```

TUI 测试也更新为普通输入通过 mock provider 返回 assistant 回复。

## 下一步建议

下一步进入：

```text
feat: add openai compatible provider skeleton
```

先不打真实网络，先完成 OpenAI-compatible request / response / tool_calls 转换和解析层。
