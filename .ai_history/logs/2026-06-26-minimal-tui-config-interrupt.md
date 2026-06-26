# AI 协作记录：Minimal TUI + Interrupt + API Config

日期：2026-06-26

## 背景

用户在 Windows PowerShell 中运行 `agent_tui.exe`，发现程序只输出：

```text
agent_tui minimal runner skeleton
```

这说明核心 Harness 已经推进，但最基础 TUI 入口还没有接上。用户要求先把 TUI、配置 API 和中断能力接上。

## 本轮技术文档

新增：

```text
docs/17-minimal-tui-config-interrupt-design.md
```

文档明确：

- 先做 TUI-lite，不引入 FTXUI。
- 先提供可运行交互入口。
- 支持 `/api`、`/model`、`/interrupt` 和基础内置命令。
- API Key 只保存环境变量名，不读取、不打印真实 key。

## 本轮实现

新增：

```text
include/agent_tui/tui/TuiConfig.hpp
include/agent_tui/tui/TuiApp.hpp
tests/test_tui_app.cpp
docs/18-minimal-tui-config-interrupt-verification.md
```

更新：

```text
src/main.cpp
CMakeLists.txt
README.md
TODO.md
```

## 当前新增能力

`agent_tui.exe` 启动后进入交互式 TUI-lite：

```text
agent_tui  |  status: IDLE
provider=<...> model=<...> api_base=<...> key_env=<...>
Chat
Commands
>
```

支持命令：

```text
/help
/status
/clear
/model
/api
/interrupt
/skills
/exit
```

支持 API 运行时配置：

```text
/api provider openai
/api base https://api.openai.com/v1
/api key-env OPENAI_API_KEY
/api timeout 60
/api max-loops 8
```

支持中断：

```text
/interrupt
Ctrl+C
```

第一版中断只设置 interrupted 标记，后续由 AgentLoop、Provider streaming 和 ShellTool 读取。

## 测试覆盖

新增测试：

```text
test_model_command_sets_model
test_api_commands_set_runtime_config_without_key_value
test_interrupt_command_sets_flag
test_clear_command_resets_history_and_interrupt
test_exit_command_stops_app
test_run_accepts_scripted_input
```

## 下一步建议

下一步进入：

```text
feat: add config loader with user project priority
```

原因：TUI 已经有运行时 `/api` 配置命令，下一步需要实现用户级 / 项目级配置加载和项目级优先级。
