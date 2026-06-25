# Agent Learning Hub 启发总结

## 1. 背景

本项目目标是实现一个纯 C++ 本地 TUI Coding Agent。近期讨论中参考了 FastClaw、Nanobot、agents-from-scratch，以及 Datawhale 的 `Agent-Learning-Hub`。

`Agent-Learning-Hub` 不是一个 Agent 框架源码，而是一份 Agent 学习路线和工程选型清单。它对本项目最大的价值在于：帮助确认当前方向应该聚焦在 **Codex / Claude Code 风格的 coding agent harness**，而不是老式 role-play 多 Agent 框架。

## 2. 核心判断

`Agent-Learning-Hub` 的主线判断可以概括为：

```text
现代 Agent 的重点不是“角色扮演”，而是 Agent Harness 工程能力。
```

对本项目来说，这意味着第一优先级不是多 Agent，也不是复杂记忆，而是：

- 工具协议
- 工具调用解析
- 权限控制
- 状态管理
- 会话回放
- Trace / Audit Log
- MockProvider 评测
- 安全边界
- Skill 能力包

## 3. 对 agent_tui 的路线启发

### 3.1 先做最小 Agent Loop

`Agent-Learning-Hub` 的 Stage 1 强调先实现一个最小 Agent Loop：

- 能调用 LLM API
- 能让模型输出结构化 tool call
- 能定义工具函数
- 能解析工具调用
- 能执行工具
- 能把工具结果回传模型
- 能加最大步数、超时和错误处理

对应到本项目，下一步不应该先做完整 TUI 或完整 Provider，而应该先实现：

```text
Provider
MockProvider
ToolRegistry
AgentRunner
Done tool
max_loops
```

第一版最小闭环：

```text
MockProvider 第一次返回 tool_call
ToolRegistry 执行 fake tool
MockProvider 第二次返回 Done
AgentRunner 停止并返回 final answer
```

### 3.2 Agent 能力主要来自 Harness

Agent-Learning-Hub 强调：Agent 的能力很大一部分来自 harness，而不是模型本身。

对本项目来说，Agent Harness 应包括：

```text
TuiApp
  ↓
AgentLoop
  ↓
AgentRunner
  ↓
Provider
  ↓
ToolRegistry
  ↓
PermissionGate
  ↓
SessionHistory / AuditLog
  ↓
SkillRuntime
```

职责拆分：

| 模块 | 职责 |
| --- | --- |
| TuiApp | 输入、输出、状态展示、权限交互 |
| AgentLoop | 用户输入、Session、SkillSelector、状态事件 |
| AgentRunner | provider/tool loop、max_loops、Done 终止 |
| Provider | MockProvider、OpenAI-compatible、CodingPlanProvider |
| ToolRegistry | 工具注册、schema 暴露、工具执行 |
| PermissionGate | Approve、Deny、Edit、Feedback |
| SessionHistory | 消息、工具调用、工具结果、权限拒绝、错误信息 |
| AuditLog | `.ai_history/logs/` 与调试 trace |
| SkillRuntime | 加载 skill.yaml / SKILL.md，选择和注入能力说明 |

### 3.3 Skills 是能力包，不是普通 Prompt

`Agent-Learning-Hub` 对 Skill 的定位很适合本项目：

```text
Tool 是可调用接口。
Skill 是可复用流程知识。
Prompt 是一次性指令。
MCP 是外部工具/数据源协议。
```

因此本项目的 Skills Runtime 不应只是 prompt 拼接，而应该具备：

- 可发现
- 可加载
- 可版本化
- 可限制工具集合
- 可提供触发条件
- 可提供使用流程
- 可提供验收标准
- 可做 smoke test

建议第一批 Skills：

```text
repo_reader
code_editor
shell_runner
cpp_project
tui_agent
kwoa_cli
```

其中 `kwoa_cli` 作为真实业务 Skill 验证对象，不只是参考资料。

### 3.4 Evaluation 和 Safety 必须前置

`Agent-Learning-Hub` 明确强调，没有 eval、trace、权限边界的 agent 只能算 demo。

因此本项目需要尽早加入：

- MockProvider 脚本化测试
- 固定测试集
- 工具调用次数记录
- 成功 / 失败原因记录
- 权限拒绝回传
- tool abuse 防护
- prompt injection 防护
- 敏感文件读取防护

最小测试建议：

```text
test_agent_runner_single_tool_call
test_agent_runner_multi_tool_call
test_done_tool_stops_loop
test_tool_not_found_goes_back_to_model
test_denied_write_file_goes_back_to_model
test_denied_run_shell_goes_back_to_model
test_permission_edit_changes_args
test_skill_selector_kwoa_cli
test_kwoa_cli_send_message_requires_confirm
test_skill_md_cannot_be_exfiltrated
```

### 3.5 第一阶段不要做 Browser / Computer Use

`Agent-Learning-Hub` 把 Browser / Computer-Use Agents 放在较后阶段，并强调截图、DOM、动作日志和安全限制。

本项目第一阶段不应做浏览器自动化。当前只保留：

```text
文件工具
文本搜索
Shell 执行
kwoa-cli 真实 Skill 验证
```

## 4. 推荐实现顺序

结合 FastClaw、Nanobot、agents-from-scratch 和 Agent-Learning-Hub，本项目路线调整为：

```text
0. 文档和方向确认：已完成
1. C++ 工程骨架
2. Minimal AgentRunner：Provider + ToolRegistry + Done
3. MockProvider 评测：固定测试集
4. File/Shell 工具：list/read/search/write/edit/run
5. PermissionGate：Approve / Deny / Edit / Feedback
6. SkillRuntime：加载 SKILL.md + skill.yaml
7. kwoa-cli Skill 验证：真实能力包 smoke test
8. TUI：展示消息、工具、权限、状态
9. CodingPlanProvider：真实模型接入
10. deliverables：截图、demo、AI logs
```

## 5. kwoa-cli Skill 验证定位

`kwoa-cli` 不应该只是被分析项目，而应作为本项目第一个真实 Skill 验证对象。

验证目标：

```text
如果 agent_tui 能加载 kwoa-cli Skill，
理解它的安全规则，
执行只读命令，
拦截写命令，
把结果回传模型继续推理，
那 Agent 内核就真的跑通了。
```

建议 smoke test：

```text
用户：用 kwoa-cli 看看最近会话，不要发送消息。

期望流程：
1. SkillSelector 命中 kwoa_cli。
2. Agent 加载 kwoa-cli 规则。
3. Agent 调用 auth status。
4. 如果未初始化，提示用户更新 cookie，停止，不猜数据。
5. 如果已初始化，执行 read-only recent-list。
6. 输出数据来源、命令、限制、结论。
```

写操作验证：

```text
用户：帮我给 chat 123 发 hello。

期望流程：
1. Agent 识别这是高风险写操作。
2. 先展示命令或 dry-run。
3. 请求用户明确确认。
4. 未确认时不执行。
5. 将拒绝作为 tool_result 回传模型。
```

## 6. 对 TODO 的更新建议

TODO 应增加以下项目：

```text
- [ ] 实现 AgentRunner，而不是只有 AgentLoop。
- [ ] 增加 Done 虚拟工具，支持明确终止。
- [ ] MockProvider 支持脚本化 tool_call 测试。
- [ ] PermissionGate 支持 Approve / Deny / Edit / Feedback。
- [ ] 新增 kwoa_cli Skill 作为真实 smoke test。
- [ ] 增加 skill exfiltration 防护测试。
- [ ] 增加固定 Agent 行为测试集。
```

## 7. 当前决策

本项目主线确定为：

```text
用 Agent-Learning-Hub 的学习/工程节奏，
实现 FastClaw / Nanobot 风格的小型 C++ Agent Harness，
再用 kwoa-cli Skill 做真实验收。
```

第一阶段不要做大平台，不要做多 Agent，不要做 Browser Agent，不要做复杂长期记忆。先把最小 AgentRunner、ToolRegistry、MockProvider、PermissionGate 和 SkillRuntime 跑通。
