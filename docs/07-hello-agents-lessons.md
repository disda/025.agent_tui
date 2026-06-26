# Hello-Agents：从零开始构建智能体的启发

## 1. 为什么参考 Hello-Agents

Datawhale 的 `hello-agents` 是一套系统性智能体学习教程，核心目标是从基础理论到实际应用，逐步掌握智能体系统的设计与实现。

对本项目来说，它的价值不是提供一套可直接照搬的 Python 代码，而是提供一条清晰的工程路线：

```text
不要只调用成熟框架 API，
而是亲手实现 Agent 的核心闭环、工具系统、消息系统、配置、错误处理和运行轨迹。
```

本项目是 C++ TUI Coding Agent，因此应吸收其设计思想，而不是照搬 Python 包结构。

## 2. 它首先说明：为什么要从零构建

Hello-Agents 第七章强调，从零构建 Agent 框架的价值在于：

- 成熟框架容易过度抽象。
- 商业框架 API 变化快。
- 黑盒实现不利于理解内部机制。
- 依赖复杂，容易与已有项目冲突。
- 自建框架能真正理解 Agent 的思考过程、工具调用机制和设计模式差异。
- 自建框架能获得完全控制权。
- 构建过程能训练模块化设计、接口抽象和错误处理能力。

对 `agent_tui` 来说，这正好对应题目要求：核心 Agent 能力必须自行实现，不能依赖第三方 Agent SDK 或 Agent Framework。

## 3. 它教我们：Agent 框架必须分层

Hello-Agents 的章节设计中，框架被拆为：

```text
core/
  agent.py
  llm.py
  message.py
  config.py
  exceptions.py

agents/
  simple_agent.py
  react_agent.py
  reflection_agent.py
  plan_solve_agent.py

tools/
  base.py
  registry.py
  chain.py
  async_executor.py
  builtin/
```

其核心原则是：

```text
分层解耦、职责单一、接口统一。
```

对应到本项目，C++ 结构应保持为：

```text
agent/
  Message
  ToolCall
  AgentResult
  AgentRunner
  AgentLoop

llm/
  Provider
  MockProvider
  OpenAICompatibleProvider
  CodingPlanProvider

tools/
  Tool
  ToolRegistry
  FileTools
  ShellTool
  KwoaCliTool

permissions/
  PermissionGate
  ApprovalService
  ApprovalDecision

workspace/
  Workspace
  WorkspaceGuard

skills/
  Skill
  SkillRegistry
  SkillSelector

session/
  SessionHistory
  AuditLog

tui/
  TuiApp
  PermissionPanel
  StatusBar
```

## 4. 它教我们：第一阶段要轻量，不要过度抽象

Hello-Agents 强调学习框架应具备完整可读性，依赖要尽量少，遇到问题可以直接定位到框架代码。

本项目第一阶段也应遵守：

```text
不要先做复杂 RAG。
不要先做多 Agent。
不要先做 MCP。
不要先做浏览器自动化。
不要先做长期记忆。
不要先做插件市场。
```

第一阶段的目标是跑通：

```text
MockProvider
  ↓
AgentRunner
  ↓
ToolRegistry
  ↓
FileTools / ShellTool
  ↓
ToolResult 回传
  ↓
Done
```

## 5. 它教我们：基于标准 API，而不是先发明复杂协议

Hello-Agents 选择在 OpenAI-compatible 风格 API 之上构建，是为了降低学习和迁移成本。

本项目也应保持 Provider 层简单：

```cpp
class Provider {
public:
    virtual ~Provider() = default;
    virtual ProviderResponse chat(const std::vector<Message>& messages) = 0;
};
```

第一阶段 Provider 顺序：

```text
MockProvider
OpenAI-compatible Provider
WPS CodingPlan Provider
```

## 6. 它教我们：渐进式开发，每一步可运行

Hello-Agents 采用版本迭代方式推进，每一步在上一版基础上增加功能，避免概念跳跃。

本项目也应按以下顺序推进：

```text
1. Minimal AgentRunner
2. FileTools + WorkspaceGuard
3. PermissionGate
4. run_shell
5. write_file / edit_file
6. SkillRuntime
7. kwoa_cli Skill smoke test
8. TUI
9. CodingPlanProvider
10. deliverables 截图和验证日志
```

## 7. 它教我们：“万物皆工具”是第一阶段最好的简化

Hello-Agents 的一个关键设计是：除了核心 Agent 类，一切皆为 Tools。

这对本项目很重要。第一阶段不要提前设计复杂的 MemorySystem、RagEngine、CommandEngine 或 DocumentEngine，而是先统一为工具：

```text
list_dir
read_file
glob_files
search_text
write_file
edit_file
run_shell
run_kwoa_cli
```

核心公式：

```text
Agent = LLM + Messages + Tools + Loop + Permission + Session
```

Skill 只负责告诉模型什么时候用、怎么用、注意什么。

## 8. 它教我们：先 ReAct，再考虑 Planner 和 Reflection

Hello-Agents 第四章讲了三类经典范式：

- ReAct：思考与行动交替。
- Plan-and-Solve：先计划再执行。
- Reflection：自我批判和修正。

对于 coding agent，第一阶段应先实现 ReAct-like loop：

```text
模型决定下一步 tool_call
  ↓
工具执行
  ↓
工具结果回传
  ↓
模型根据结果继续
```

原因是 coding agent 的下一步经常依赖工具结果：

```text
读完文件才知道要改哪里。
跑完测试才知道失败在哪里。
auth status 之后才知道能不能查 kwoa 消息。
```

Plan-and-Solve 和 Reflection 可以作为后续增强：

```text
CommandPlanner = Plan-and-Solve 的工程化版本。
测试失败后的修正 = Reflection 的工程化版本。
```

## 9. 它教我们：Agent 的核心是与外部世界交互

现代智能体的核心能力不是聊天，而是让模型通过工具连接外部世界：

```text
读文件
搜索代码
执行命令
调用 API
写文件
运行测试
观察结果
继续推理
```

因此本项目必须重点实现：

- ToolRegistry
- WorkspaceGuard
- PermissionGate
- FileTools
- run_shell
- SessionHistory
- AuditLog
- max_loops
- Done tool

这些不是附加功能，而是 Agent 是否能安全工作的前提。

## 10. 对 agent_tui 的直接结论

本项目不是：

```text
kwoa-cli 专用 Agent
```

而是：

```text
通用 C++ Agent Harness + 可插拔 Skills
```

其中 `kwoa-cli` 是第一个真实 Skill 验证场景。

正确路线：

```text
先做通用 Harness，
再用 kwoa-cli Skill 验证 Harness 是否真实可用。
```

## 11. 下一步行动

根据 Hello-Agents 的启发，下一步继续做通用地基：

```text
feat: add file tools and workspace guard
```

原因：

```text
ReAct 需要 Observation。
Observation 来自工具。
最基础的工具就是文件读取和搜索。
文件工具必须先有 WorkspaceGuard。
```

后续依次推进：

```text
feat: add permission gate
feat: add controlled shell tool
feat: add skill runtime
feat: add kwoa-cli skill smoke test
```
