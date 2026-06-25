# AI 协作记录：Agent Learning Hub 启发总结

日期：2026-06-26

## 背景

用户提供 `https://github.com/datawhalechina/Agent-Learning-Hub`，希望分析它对当前 `agent_tui` 项目的启发，并将总结沉淀到文档中。

## 参考结论

`Agent-Learning-Hub` 不是 Agent 框架源码，而是一份 Agent 学习路线和工程选型清单。它对本项目最重要的启发是：当前主线应该聚焦 Codex / Claude Code 风格 coding agent harness，而不是老式 role-play 多 Agent。

## 本轮新增文档

新增：

```text
docs/04-agent-learning-hub-insights.md
```

文档总结了以下内容：

- 先做最小 Agent Loop / AgentRunner。
- Agent 能力主要来自 Harness，而不是模型本身。
- Skills 是能力包，不是普通 Prompt。
- Evaluation 和 Safety 必须前置。
- 第一阶段不做 Browser / Computer Use。
- kwoa-cli Skill 作为真实 smoke test。

## TODO 更新

更新：

```text
TODO.md
```

主要调整：

- 新增 Agent-Learning-Hub 启发章节。
- 将下一步从泛泛的 AgentLoop 调整为 Minimal AgentRunner。
- 明确加入 Done 虚拟工具。
- PermissionGate 预留 Approve / Deny / Edit / Feedback。
- MockProvider 增加固定行为测试集。
- Skills Runtime 增加 `kwoa_cli`。
- 新增 kwoa-cli Skill 验证章节。

## 当前路线

```text
用 Agent-Learning-Hub 的学习/工程节奏，
实现 FastClaw / Nanobot 风格的小型 C++ Agent Harness，
再用 kwoa-cli Skill 做真实验收。
```

## 下一步建议

下一次提交建议直接进入代码骨架：

```text
feat: add minimal agent runner skeleton
```

包含：

- `CMakeLists.txt`
- `src/main.cpp`
- `include/agent_tui/agent/Message.hpp`
- `include/agent_tui/agent/ToolCall.hpp`
- `include/agent_tui/agent/AgentRunner.hpp`
- `include/agent_tui/tools/Tool.hpp`
- `include/agent_tui/tools/ToolRegistry.hpp`
- `include/agent_tui/llm/Provider.hpp`
- `include/agent_tui/llm/MockProvider.hpp`
- `tests/test_agent_runner.cpp`
