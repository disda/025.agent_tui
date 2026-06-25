# AI 协作记录：Minimal AgentRunner Skeleton

日期：2026-06-26

## 背景

用户要求开始构建项目地基，并询问是否可以在虚拟/隔离环境中运行项目。

本项目是 C++ 项目，因此没有 Python venv。本轮使用独立 `build/` 目录作为隔离构建环境，在本地沙箱验证 CMake 构建和 CTest 通过。

## 本轮实现

新增最小 C++ AgentRunner 骨架：

```text
CMakeLists.txt
src/main.cpp
include/agent_tui/agent/Message.hpp
include/agent_tui/agent/ToolCall.hpp
include/agent_tui/agent/AgentResult.hpp
include/agent_tui/agent/AgentRunner.hpp
include/agent_tui/llm/Provider.hpp
include/agent_tui/llm/MockProvider.hpp
include/agent_tui/tools/Tool.hpp
include/agent_tui/tools/ToolRegistry.hpp
tests/test_agent_runner.cpp
```

同时更新：

```text
.gitignore
README.md
TODO.md
docs/06-build-verification.md
```

## 当前能力

AgentRunner 当前支持：

- Provider 返回普通文本，直接完成。
- Provider 返回 tool_call，ToolRegistry 查找并执行工具。
- 工具结果作为 tool role message 回传。
- `Done` 虚拟工具终止循环。
- tool not found 时将错误作为 tool_result 回传。
- max_loops 超限时返回失败。

MockProvider 当前支持：

- 脚本化返回 ProviderResponse。
- 记录最后一次观察到的 messages。
- 统计调用次数。

测试覆盖：

- 单个 tool_call 后 Done。
- tool not found 回传模型。
- max_loops exceeded。

## 本地隔离验证

执行命令：

```bash
cmake -S /mnt/data/agent_tui_skel -B /mnt/data/agent_tui_skel/build
cmake --build /mnt/data/agent_tui_skel/build
ctest --test-dir /mnt/data/agent_tui_skel/build --output-on-failure
```

结果：

```text
100% tests passed, 0 tests failed out of 1
```

## 下一步建议

下一步建议实现真实文件工具和 workspace guard：

```text
feat: add file tools and workspace guard
```

优先文件：

```text
include/agent_tui/tools/FileTools.hpp
include/agent_tui/workspace/Workspace.hpp
tests/test_file_tools.cpp
```

目标：让 AgentRunner 从 fake echo tool 进入真实 workspace 文件读取能力。
