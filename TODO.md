# TODO：C++ TUI Agent 下一步计划

> 当前项目方向：纯 C++ 实现一个本地 TUI Coding Agent，并采用 kwoa-cli 风格 Skills Runtime 组织能力。

## 当前状态

- [x] 明确项目从 Python/Textual 方案调整为纯 C++ 方案。
- [x] 明确采用 kwoa-cli 风格 Skills Runtime。
- [x] 完成设计文档：`docs/00` 到 `docs/18`。
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
- [x] 新增 TUI `/api` 运行时配置命令。
- [x] 新增 TUI `/interrupt` 中断标记。
- [x] 新增 `tests/test_agent_runner.cpp`。
- [x] 新增 `tests/test_file_tools.cpp`。
- [x] 新增 `tests/test_permission_gate.cpp`。
- [x] 新增 `tests/test_shell_tool.cpp`。
- [x] 新增 `tests/test_write_edit_tools.cpp`。
- [x] 新增 `tests/test_session_history.cpp`。
- [x] 新增 `tests/test_tui_app.cpp`。
- [x] 创建 `deliverables/` 可运行验证产物目录。
- [x] 明确最终验收 Demo：加载 kwoa-cli Skill，实现 IM / KDocs 文档操作能力验证。
- [x] 明确需要受控代码/脚本执行能力，但第一版只做受控 Shell，不做完整解释器或沙箱。

## 作业要求对齐状态

| 作业要求 | 当前状态 | 还缺什么 |
| --- | --- | --- |
| 可编译运行的 TUI Agent 完整源码 | 部分完成 | 还缺 AgentLoop 应用层、真实 Provider、配置系统 |
| TUI 展示用户输入、模型回复、工具调用、工具结果、权限确认、当前状态 | 部分完成 | 已有 TUI-lite、输入、状态、配置命令；还缺 Tool Log、Permission Panel、AgentLoop 集成 |
| 模型决策 -> 工具调用 -> 结果回传 -> 继续推理 | 部分完成 | `AgentRunner` 已具备核心闭环并接入 SessionHistory；还缺 AgentLoop、真实 Provider 接入 |
| 仓库工具：目录浏览、文件读取、文件匹配、内容搜索 | 已完成 | 后续优化编码检测、大文件处理 |
| 文件写入或编辑 | 已完成 | 后续可增强 diff/patch、AST 级编辑 |
| Shell 命令执行 | 已完成 | 后续可增强命令风险策略和 Windows 支持 |
| 权限控制：写文件、编辑、Shell 必须确认 | 已完成核心机制 | 后续 TUI 中需要可视化确认面板 |
| 用户拒绝授权不得执行，拒绝结果进入上下文 | 已完成核心机制 | PermissionGate 已做到拒绝不执行，SessionHistory 已记录 permission_denied / tool_result |
| WPS CodingPlan 协议 | 未完成 | 需要 `CodingPlanProvider`、工具调用解析、流式输出、超时、重试 |
| 多轮会话上下文管理 | 部分完成 | 已有 SessionHistory / AuditLog；还缺 AgentLoop 多轮会话组织和 TUI 展示 |
| 用户级 / 项目级配置，项目级优先 | 未完成 | 需要 `ConfigLoader`，覆盖 Provider、模型、API 地址、超时、max_loops，API Key 脱敏 |
| TUI 内置命令 | 部分完成 | 已有 `/help`、`/clear`、`/model`、`/status`、`/exit`、`/skills`、`/api`、`/interrupt`；还缺接 AgentLoop 后的真实状态 |
| 不使用第三方 Agent SDK / Framework | 符合 | 继续保持核心 AgentLoop、工具、权限、会话自行实现 |
| `.ai_history/logs/` 必须提交 | 已完成并持续补充 | 每轮关键设计继续补日志 |
| `deliverables/` 可运行验证产物和截图 | 部分完成 | 目录和计划已创建，还缺真实运行日志和关键截图 |
| 测试至少覆盖 Agent 主循环、工具调用结果回传、权限确认与拒绝、配置优先级、Mock LLM Provider | 部分完成 | 已覆盖 AgentRunner、工具、权限、MockProvider、会话记录、TUI 命令；还缺配置优先级测试 |

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
```

## 交付清单状态

| 交付要求 | 状态 | 位置 |
| --- | --- | --- |
| 项目源码仓库 | 进行中 | `CMakeLists.txt`、`include/`、`src/`、`tests/` |
| AI 协作过程记录 | 进行中，已提交 | `.ai_history/logs/` |
| 可运行验证产物 | 已创建目录，持续补充 | `deliverables/` |

当前 deliverables：

```text
deliverables/README.md
deliverables/demo-kwoa-cli-skill.md
deliverables/screenshots/.gitkeep
```

## 项目最终验收目标

本项目最终不是做普通玩具 Demo，而是验证一个能加载真实 Skill 的本地 Coding Agent Harness：

```text
加载 kwoa-cli Skill，
理解 IM / KDocs 文档操作规则，
执行只读 IM / 文档查询，
对消息发送、撤回、转发、reaction、KDocs 写入进行权限确认，
把工具结果回传模型继续推理。
```

## 下一步优先级

### 1. ConfigLoader / 配置优先级

TUI 已经支持运行时 `/api` 配置命令，下一步要补用户级 / 项目级配置加载和测试。

- [ ] `Config` 数据结构。
- [ ] 用户级配置路径：`~/.agent-tui/config.yaml`。
- [ ] 项目级配置路径：`./.agent-tui.yaml`。
- [ ] 项目级配置覆盖用户级配置。
- [ ] 环境变量读取 API Key。
- [ ] API Key 脱敏输出。
- [ ] 覆盖 Provider、模型、API 地址、超时时间、最大循环轮次。
- [ ] `test_project_config_overrides_user_config`。
- [ ] `test_api_key_is_not_exposed`。

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
- [ ] IM 只读命令限制 count/page-limit。
- [ ] KDocs 读取命令支持 docs +info / docs +read。
- [ ] IM / KDocs 写操作必须走 PermissionGate。
- [ ] 增加 `test_kwoa_cli_send_message_requires_confirm`。

### 4. Minimal TUI 增强

当前已有 TUI-lite，后续增强为更接近题目要求的结构化展示。

- [ ] Tool Call Log。
- [ ] Tool Result Log。
- [ ] Permission Panel。
- [ ] AgentLoop 状态展示：Idle / Thinking / CallingTool / WaitingApproval / RunningTool / Done / Failed。
- [ ] 流式输出区域。

### 5. Provider / CodingPlan

先补 OpenAI-compatible Provider，再补 WPS CodingPlan Provider。

- [ ] `OpenAICompatibleProvider`。
- [ ] `CodingPlanProvider`。
- [ ] 工具调用解析。
- [ ] 流式输出。
- [ ] 超时控制。
- [ ] 基础重试。
- [ ] Provider 错误回传。

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
feat: add minimal TUI config and interrupt commands
```

已包含：

- `docs/17-minimal-tui-config-interrupt-design.md`
- `include/agent_tui/tui/TuiConfig.hpp`
- `include/agent_tui/tui/TuiApp.hpp`
- `tests/test_tui_app.cpp`

已实现：

- `agent_tui.exe` 启动进入交互式 TUI-lite。
- `/help`。
- `/status`。
- `/clear`。
- `/model`。
- `/api provider`。
- `/api base`。
- `/api key-env`。
- `/api timeout`。
- `/api max-loops`。
- `/interrupt`。
- `/skills`。
- `/exit`。
- Ctrl+C 中断标记。

## 下一次最建议做的提交

```text
feat: add config loader with user project priority
```

建议包含：

- `docs/19-config-loader-design.md`
- `include/agent_tui/config/Config.hpp`
- `include/agent_tui/config/ConfigLoader.hpp`
- `tests/test_config_loader.cpp`

目标：满足交付要求和测试范围中对用户级配置、项目级配置、项目优先级、敏感信息不暴露的要求。

## 当前不要做

第一阶段先不要做：

- 向量数据库
- 多 Agent
- MCP
- 插件市场
- 自动 Git 提交
- Browser / Computer Use Agent
- 内置 Python / JS 解释器
- Docker / 云沙箱
- 复杂长期记忆
- 复杂上下文压缩

先把 C++ 工程骨架、MockProvider、AgentRunner、ToolSystem、PermissionGate、受控 Shell、写入编辑工具、SessionHistory、TUI-lite、配置系统、SkillRuntime、Provider 跑通。
