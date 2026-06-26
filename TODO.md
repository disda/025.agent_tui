# TODO：C++ TUI Agent 下一步计划

> 当前项目方向：纯 C++ 实现一个本地 TUI Coding Agent，并采用 kwoa-cli 风格 Skills Runtime 组织能力。

## 当前状态

- [x] 明确项目从 Python/Textual 方案调整为纯 C++ 方案。
- [x] 明确采用 kwoa-cli 风格 Skills Runtime。
- [x] 完成设计文档：`docs/00` 到 `docs/22`。
- [x] 按项目要求补充 `.ai_history/logs/` AI 协作记录。
- [x] 新增 C++ 工程骨架。
- [x] 新增 Minimal AgentRunner。
- [x] 新增 Tool / ToolRegistry 抽象。
- [x] 新增 Provider / MockProvider 抽象。
- [x] 新增 Done 虚拟工具处理。
- [x] 新增 FileTools + WorkspaceGuard。
- [x] 新增 PermissionGate。
- [x] 新增 Controlled Shell Tool。
- [x] 新增 Write / Edit Tools。
- [x] 新增 SessionHistory / AuditLog。
- [x] 新增 Minimal TUI-lite 入口。
- [x] 新增类似 `.codex` 的 TOML 配置目录。
- [x] 新增 ConfigLoader 和项目级覆盖用户级配置。
- [x] 新增 ProviderFactory 和 mock terminal chat。
- [x] 新增 OpenAI-compatible Provider 文本对话接入。
- [x] 新增 `tests/test_openai_compatible_provider.cpp`。
- [x] 创建 `deliverables/` 可运行验证产物目录。

## 作业要求对齐状态

| 作业要求 | 当前状态 | 还缺什么 |
| --- | --- | --- |
| 可编译运行的 TUI Agent 完整源码 | 部分完成 | 还缺 AgentLoop 应用层、工具调用接入真实 Provider |
| TUI 展示用户输入、模型回复、工具调用、工具结果、权限确认、当前状态 | 部分完成 | 已有 TUI-lite、输入、状态、配置命令、Provider 对话；还缺 Tool Log、Permission Panel、AgentLoop 集成 |
| 模型决策 -> 工具调用 -> 结果回传 -> 继续推理 | 部分完成 | `AgentRunner` 已具备核心闭环；OpenAI-compatible 目前只支持文本，还缺 tool_calls 解析 |
| 仓库工具：目录浏览、文件读取、文件匹配、内容搜索 | 已完成 | 后续优化编码检测、大文件处理 |
| 文件写入或编辑 | 已完成 | 后续可增强 diff/patch、AST 级编辑 |
| Shell 命令执行 | 已完成 | 后续可增强命令风险策略和 Windows 支持 |
| 权限控制：写文件、编辑、Shell 必须确认 | 已完成核心机制 | 后续 TUI 中需要可视化确认面板 |
| 用户拒绝授权不得执行，拒绝结果进入上下文 | 已完成核心机制 | PermissionGate 已做到拒绝不执行，SessionHistory 已记录 permission_denied / tool_result |
| WPS CodingPlan 协议 | 未完成 | 需要 `CodingPlanProvider`、工具调用解析、流式输出、超时、重试 |
| 多轮会话上下文管理 | 部分完成 | 已有 SessionHistory / AuditLog；还缺 AgentLoop 多轮会话组织和 TUI 展示 |
| 用户级 / 项目级配置，项目级优先 | 已完成基础版 | 后续可增强配置保存命令 |
| TUI 内置命令 | 部分完成 | 已有 `/help`、`/clear`、`/model`、`/status`、`/exit`、`/skills`、`/api`、`/config`、`/interrupt`；还缺接 AgentLoop 后的真实状态 |
| 不使用第三方 Agent SDK / Framework | 符合 | 继续保持核心 AgentLoop、工具、权限、会话自行实现 |
| `.ai_history/logs/` 必须提交 | 已完成并持续补充 | 每轮关键设计继续补日志 |
| `deliverables/` 可运行验证产物和截图 | 部分完成 | 目录和计划已创建，还缺真实运行日志和关键截图 |
| 测试至少覆盖 Agent 主循环、工具调用结果回传、权限确认与拒绝、配置优先级、Mock LLM Provider | 基础覆盖完成 | 已覆盖 AgentRunner、工具、权限、MockProvider、会话记录、TUI 命令、配置优先级；后续补真实 Provider tool_calls 测试 |

## 当前已验证命令

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

验证记录见：

```text
docs/06-build-verification.md
docs/08-file-tools-workspace-guard.md
docs/10-permission-gate-verification.md
docs/12-controlled-shell-tool-verification.md
docs/14-write-edit-tools-verification.md
docs/16-session-history-audit-log-verification.md
docs/18-minimal-tui-config-interrupt-verification.md
docs/20-config-provider-terminal-chat-verification.md
docs/22-openai-compatible-provider-verification.md
```

## 交付清单状态

| 交付要求 | 状态 | 位置 |
| --- | --- | --- |
| 项目源码仓库 | 进行中 | `CMakeLists.txt`、`include/`、`src/`、`tests/` |
| AI 协作过程记录 | 进行中，已提交 | `.ai_history/logs/` |
| 可运行验证产物 | 已创建目录，持续补充 | `deliverables/` |

## 下一步优先级

### 1. OpenAI-compatible tool_calls 解析

当前 OpenAI-compatible Provider 已能做真实文本对话，下一步要让模型能调用工具。

- [ ] ToolSchema 结构。
- [ ] ToolRegistry 导出 ToolSchema。
- [ ] OpenAI-compatible tools JSON 构造。
- [ ] 解析 `tool_calls`。
- [ ] 将 tool_calls 转为内部 `ToolCall`。
- [ ] AgentRunner + OpenAI-compatible Provider 真实工具调用闭环。
- [ ] `tests/test_openai_compatible_tool_calls.cpp`。

### 2. AgentLoop 应用层

`AgentRunner` 已有核心 tool loop，TUI-lite 已有输入入口，但还缺面向应用的 AgentLoop。

- [ ] 接收用户输入。
- [ ] 处理内置命令。
- [ ] 调用 SkillSelector。
- [ ] 构造 messages。
- [ ] 调用 AgentRunner。
- [ ] 把事件写入 SessionHistory / AuditLog。
- [ ] 将运行状态推给 TUI。

### 3. SkillRuntime + kwoa-cli Skill 验证

实现通用 SkillRuntime，再用 `kwoa_cli` 作为第一个真实 Skill 验证场景。

- [ ] 加载 `skills/*/skill.yaml`。
- [ ] 加载 `skills/*/SKILL.md`。
- [ ] SkillSelector 关键词匹配。
- [ ] 按 Skill 限制可用工具集合。
- [ ] 新增 `skills/kwoa_cli/skill.yaml`。
- [ ] 新增 `skills/kwoa_cli/SKILL.md`。
- [ ] 新增 `run_kwoa_cli` 工具或通过 `run_shell` 安全封装。
- [ ] `auth status` 作为只读初始化检查。
- [ ] IM / KDocs 写操作必须走 PermissionGate。

### 4. CodingPlan Provider Skeleton

作业明确要求 WPS CodingPlan 协议，需单独 Provider。

- [ ] `CodingPlanProvider`。
- [ ] 工具调用解析。
- [ ] 流式输出。
- [ ] 超时控制。
- [ ] 基础重试。
- [ ] Provider 错误回传。

### 5. Minimal TUI 增强

当前已有 TUI-lite，后续增强为更接近题目要求的结构化展示。

- [ ] Tool Call Log。
- [ ] Tool Result Log。
- [ ] Permission Panel。
- [ ] AgentLoop 状态展示：Idle / Thinking / CallingTool / WaitingApproval / RunningTool / Done / Failed。
- [ ] 流式输出区域。

### 6. Deliverables 真实验证产物

最后阶段补可运行验证产物和截图。

- [ ] `deliverables/run-log.md`。
- [ ] `deliverables/demo-task.md`。
- [ ] `deliverables/screenshots/01-tui-start.png`。
- [ ] `deliverables/screenshots/02-tool-call.png`。
- [ ] `deliverables/screenshots/03-permission-confirm.png`。
- [ ] `deliverables/screenshots/04-test-run.png`。
- [ ] `deliverables/screenshots/05-final-answer.png`。

## 刚完成的提交

```text
feat: add openai compatible provider
```

已包含：

- `docs/21-openai-compatible-provider-design.md`。
- `include/agent_tui/llm/OpenAICompatibleProvider.hpp`。
- `tests/test_openai_compatible_provider.cpp`。
- `docs/22-openai-compatible-provider-verification.md`。

已实现：

- OpenAI-compatible 文本对话。
- 从 `api_key_env` 读取 API Key。
- 使用 `api_base + /chat/completions`。
- 构造 messages JSON。
- 解析 assistant text response。
- 解析 provider error response。
- ProviderFactory 接入 `openai-compatible` 和 `openai`。

## 下一次最建议做的提交

```text
feat: add openai compatible tool call parsing
```

目标：让真实 Provider 能调用本地工具，进入完整 Coding Agent 闭环。
