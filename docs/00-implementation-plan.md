# C++ TUI Coding Agent 中文实施计划

> 当前计划以 L2 题目为准：先实现一个可运行的 TUI Coding Agent。主线是模型驱动工具调用，不再把规则 Intent、Skills Runtime、kwoa-cli 或桌面文件管理放在当前优先级。

## 1. 目标

从零实现一个最小可用的本地 TUI 编码 Agent。接入 API 后，用户输入简单开发任务，Agent 能自主读取仓库、写入代码、执行命令、根据工具结果继续推理，并给出最终答复。

第一阶段验收目标：

```text
用户输入：实现一个简单代码 demo
  -> Provider 返回 write_file tool_call
  -> TUI 请求写文件确认
  -> ToolRegistry 执行 write_file
  -> Provider 看到 tool_result 后继续
  -> Provider 返回 run_shell tool_call
  -> TUI 请求命令确认
  -> ToolRegistry 执行 run_shell
  -> Provider 看到执行结果后返回最终答案
```

## 2. 当前架构主线

```text
TUI
  -> AgentRunner
  -> Provider
  -> ToolCall parser
  -> Permission / ApprovalService
  -> ToolRegistry
  -> SessionHistory / AuditLog
  -> Provider 继续推理
```

约束：

- Agent Loop、工具系统、权限控制、会话管理自行实现。
- Provider 只负责模型协议，不执行工具。
- Tool 是唯一执行入口。
- `write_file`、`edit_file`、`run_shell` 必须确认。
- Local Intent Router 只作为临时快捷命令，不承担自然语言理解主路径。

## 3. 当前已完成

- C++20 + CMake 工程。
- TUI-lite 主入口。
- 配置加载：用户级 + 项目级，项目级优先。
- Provider 抽象、MockProvider、OpenAI-compatible Provider 基础版。
- AgentRunner 工具循环。
- ToolRegistry。
- 文件只读工具：`list_dir`、`read_file`、`glob_files`、`search_text`。
- 写编辑工具：`write_file`、`edit_file`。
- Shell 工具：`run_shell`。
- 权限确认抽象：ApprovalService。
- SessionHistory / AuditLog。
- TUI 第一版已接入 AgentRunner。
- `mock-agent-demo` 已能通过 TUI tool loop 写出 `demo.py`。

## 4. P0 实施计划：代码 Demo 闭环

### P0-A：TUI 接入 AgentRunner

状态：已完成第一版。

已完成内容：

- TUI 普通用户输入进入 AgentRunner。
- TUI 注册基础仓库工具集合。
- TUI ApprovalService 能对 `write_file` 进行确认。
- 新增测试覆盖 `mock-agent-demo` 通过 TUI 生成 `demo.py`。

下一步：

- 在 TUI 消息区显示 tool_call 参数摘要。
- 在 TUI 消息区显示 tool_result。
- 状态栏显示 THINKING / WAITING_APPROVAL / RUNNING_TOOL / DONE / ERROR。

### P0-B：Demo 覆盖写文件和运行命令

目标：

```text
/api provider mock-agent-demo
实现一个 hello world Python demo，并运行它
```

期望结果：

- 生成 `demo.py`。
- 用户确认写文件。
- 用户确认 `python3 demo.py`。
- 输出包含 `hello from agent_tui`。
- 最终回答说明 demo 已完成。

任务：

- [ ] 扩展 `mock-agent-demo`，让它依次返回 `write_file`、`run_shell`、`Done`。
- [ ] 增加 TUI 测试，脚本输入包含两次 `y`。
- [ ] 确认输出中包含工具执行结果。

### P0-C：真实 API tool_calls

目标：把 `mock-agent-demo` 的路径换成真实 OpenAI-compatible API。

任务：

- [ ] ToolRegistry 导出 tools schema。
- [ ] OpenAI-compatible Provider 请求体从 ToolRegistry 获取 tools schema。
- [ ] OpenAI-compatible Provider 解析真实 `tool_calls`。
- [ ] 修正 tool result message 格式，满足 OpenAI-compatible API 要求。
- [ ] 运行一次真实 API demo。
- [ ] 把运行记录写入 `deliverables/run-log.md`。

## 5. P1：TUI 可观察性

题目明确要求 TUI 展示过程信息，因此需要补：

- [ ] 用户输入。
- [ ] 模型回复。
- [ ] tool_call。
- [ ] tool_result。
- [ ] 权限确认。
- [ ] 当前运行状态。
- [ ] 错误信息。

第一版可以继续保持 TUI-lite，不急于引入复杂 FTXUI 布局。

## 6. P2：WPS CodingPlan Provider

OpenAI-compatible 主闭环稳定后，再实现题目要求的 CodingPlan Provider：

- [ ] `CodingPlanProvider`。
- [ ] tool_call 解析。
- [ ] 流式输出。
- [ ] 超时控制。
- [ ] 基础重试。
- [ ] 错误进入 SessionHistory。

## 7. P3：后续扩展

以下内容不是当前主线：

- Skills Runtime。
- kwoa-cli Skill。
- 桌面文件整理。
- 多 Agent。
- 向量数据库。
- 上下文压缩。

它们可以作为扩展加分项，但必须在基础 Coding Agent demo 闭环完成之后再推进。

## 8. 验证命令

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

重点测试：

- `agent_tui_app_tests`
- `agent_tui_tests`
- `agent_tui_openai_compatible_provider_tests`
- `agent_tui_permission_gate_tests`
- `agent_tui_write_edit_tools_tests`
- `agent_tui_shell_tool_tests`
