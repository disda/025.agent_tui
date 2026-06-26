# 验证说明：Minimal TUI + Interrupt + API Config

日期：2026-06-26

## 1. 验证目标

本次验证目标是让 `agent_tui.exe` 不再只输出 skeleton，而是启动一个最小可交互终端界面。

本轮接入：

```text
Minimal TUI
/api 配置命令
/model 模型切换命令
/interrupt 中断标记
/help /status /clear /skills /exit
```

## 2. 新增内容

新增：

```text
include/agent_tui/tui/TuiConfig.hpp
include/agent_tui/tui/TuiApp.hpp
tests/test_tui_app.cpp
```

更新：

```text
src/main.cpp
CMakeLists.txt
```

## 3. 当前能力

### TUI-lite 界面

程序启动后展示：

```text
agent_tui  |  status: IDLE
provider=<...> model=<...> api_base=<...> key_env=<...>
Chat
Commands
>
```

当前版本先使用纯 C++ 标准库实现，不引入 FTXUI。

### 内置命令

当前支持：

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

### API / 模型运行时配置

支持：

```text
/model <name>
/api provider <name>
/api base <url>
/api key-env <ENV_NAME>
/api timeout <seconds>
/api max-loops <n>
```

敏感信息策略：

```text
只保存 API Key 的环境变量名，不读取、不打印真实 key。
```

### 中断能力

支持：

```text
/interrupt
Ctrl+C
```

第一版中断只设置 interrupted 标记，不强杀进程。后续 AgentLoop、Provider streaming、ShellTool 长任务会读取这个标记。

## 4. 测试覆盖

`tests/test_tui_app.cpp` 覆盖：

- `/model` 能设置模型。
- `/api provider` 能设置 Provider。
- `/api base` 能设置 API base。
- `/api key-env` 只保存 env 名。
- `/api timeout` 能设置超时。
- `/api max-loops` 能设置最大循环轮次。
- `/interrupt` 能设置中断标记。
- `/clear` 能清空会话历史和中断状态。
- `/exit` 能停止 TUI。
- `run()` 能接受脚本化输入。

## 5. 验证命令

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 6. 当前限制

第一版限制：

- 还不是真正的全屏 TUI。
- 暂未接入 AgentLoop。
- 暂未接入真实 Provider。
- 暂未接入 TUI PermissionPanel。
- 暂未保存配置到文件。
- 暂未实现流式输出区域。

## 7. 下一步

下一步建议继续实现：

```text
feat: add config loader with user project priority
```

原因：TUI 已经有运行时配置命令，接下来要把用户级 / 项目级配置和 API Key 脱敏加载补上。
