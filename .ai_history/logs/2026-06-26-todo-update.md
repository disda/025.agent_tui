# AI 协作记录：TODO 更新与下一步讨论计划

日期：2026-06-26

## 背景

用户要求更新 TODO，并明确下一步应该讨论什么。此前项目已经完成 C++ + kwoa-cli 风格 Skills Runtime 的方向确认、设计文档提交和 AI 协作记录补充。

## 本轮协作动作

1. 检查仓库是否已有 TODO 相关文件。
2. 未发现现成 TODO 文件后，新增 `TODO.md`。
3. 在 README 的文档入口中加入 `TODO.md` 链接。
4. 按项目要求，将本轮关键协作记录写入 `.ai_history/logs/`。

## 新增 TODO 的核心内容

`TODO.md` 记录了当前状态和下一轮要讨论的主题：

- C++ 工程骨架
- 最小 Agent Loop 数据结构
- Tool System 最小闭环
- Permission Gate
- MockProvider
- Skills Runtime 第一版
- TUI 最小界面

## 下一轮建议讨论

下一轮建议优先讨论并提交：

```text
feat: add initial C++ project skeleton
```

该提交应包含：

- `CMakeLists.txt`
- `src/main.cpp`
- `include/agent_tui/agent/Message.hpp`
- `include/agent_tui/agent/ToolCall.hpp`
- `include/agent_tui/tools/Tool.hpp`
- `include/agent_tui/llm/Provider.hpp`
- `tests/test_placeholder.cpp`

目标是先让项目具备可编译、可测试的 C++ 骨架，而不是一次性实现完整 Agent。

## 暂不建议推进的内容

第一阶段先不要做：

- 向量数据库
- 多 Agent
- MCP
- 插件市场
- 自动 Git 提交
- 复杂长期记忆
- 完整真实 Provider

优先保证 C++ 工程骨架、MockProvider、AgentLoop、ToolSystem 和 PermissionGate 跑通。
