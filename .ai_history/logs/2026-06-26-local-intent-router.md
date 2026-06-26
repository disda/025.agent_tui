# AI 协作记录：Local Intent Router

日期：2026-06-26

## 背景

用户反馈当前很多任务无法执行，需要提升意图识别准确率。

当前 OpenAI-compatible Provider 虽然已能文本对话，但正式 tool_calls / AgentLoop 闭环仍在建设中，因此常见本地任务会出现“只回答，不执行”的问题。

## 本轮决策

在完整 Provider tool_calls 接入前，先增加本地规则型 Intent Router，作为 TUI 入口的快捷执行层：

```text
用户输入
  ↓
Local Intent Router
  ↓
命中常见本地任务：直接调用本地工具
未命中：继续走 Provider 对话
```

## 本轮技术文档

新增：

```text
docs/23-local-intent-router-design.md
docs/24-local-intent-router-verification.md
```

## 本轮实现

新增：

```text
include/agent_tui/intent/Intent.hpp
include/agent_tui/intent/IntentClassifier.hpp
tests/test_intent_classifier.cpp
```

更新：

```text
include/agent_tui/tui/TuiApp.hpp
include/agent_tui/tools/ShellTool.hpp
CMakeLists.txt
README.md
TODO.md
```

## 当前新增能力

支持常见本地意图：

```text
ls / dir / list -> list_dir
read <file> / 读取 <file> -> read_file
search <query> / 搜索 <query> -> search_text
configure -> cmake -S . -B build
build / 编译 -> cmake --build build
test / 运行测试 -> ctest --test-dir build --output-on-failure
```

Shell 类意图执行前会提示确认：

```text
Approve run_shell: <command> ? [y/N]
```

## Windows 处理

此前 `ShellTool` 在 Windows 下返回 unsupported。本轮补了 Windows `_popen` 简版实现：

```text
cmd /C "cd /d <cwd> && <command> 2>&1"
```

当前限制：

- Windows 下 stdout/stderr 合并。
- Windows 下暂不支持 timeout 强杀。

## 下一步建议

下一步继续：

```text
feat: add openai compatible tool call loop
```

目标是从本地规则型 Intent Router 过渡到模型 tool_calls 驱动的完整 AgentLoop。
