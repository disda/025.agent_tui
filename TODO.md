# TODO：C++ TUI Agent 下一步计划

> 当前项目方向：纯 C++ 实现一个本地 TUI Coding Agent，并采用 kwoa-cli 风格 Skills Runtime 组织能力。

## 当前状态

- [x] 明确项目从 Python/Textual 方案调整为纯 C++ 方案。
- [x] 明确采用 kwoa-cli 风格 Skills Runtime。
- [x] 更新 `README.md`，说明 C++ + Skills 项目方向。
- [x] 重写 `docs/00-implementation-plan.md`。
- [x] 新增 `docs/01-skill-standard.md`。
- [x] 新增 `docs/02-agent-loop-and-runtime.md`。
- [x] 新增 `docs/03-delivery-roadmap.md`。
- [x] 新增 `docs/04-agent-learning-hub-insights.md`。
- [x] 按项目要求补充 `.ai_history/logs/` AI 协作记录。

## Agent-Learning-Hub 启发

- [x] 确认主线不是老式 role-play 多 Agent，而是 Codex / Claude Code 风格 coding agent harness。
- [x] 确认 Agent 能力主要来自 harness：工具协议、权限、状态、反馈、回放、CI、评测。
- [x] 确认 Skills 是能力包，不是普通 Prompt。
- [x] 确认 Evaluation 和 Safety 需要前置，不应等到后期补。
- [x] 确认第一阶段不做 Browser / Computer Use Agent。
- [x] 确认 kwoa-cli Skill 作为真实 smoke test。

后续实现主线调整为：

```text
用 Agent-Learning-Hub 的学习/工程节奏，
实现 FastClaw / Nanobot 风格的小型 C++ Agent Harness，
再用 kwoa-cli Skill 做真实验收。
```

## 下一轮优先讨论

### 1. C++ 工程骨架

需要确定并提交第一版可编译工程：

- [ ] `CMakeLists.txt`
- [ ] `src/main.cpp`
- [ ] `include/agent_tui/...`
- [ ] `tests/...`
- [ ] `cmake -S . -B build` 可运行
- [ ] `cmake --build build` 可运行
- [ ] `ctest --test-dir build` 可运行

下一轮要重点讨论：

- 是否使用 FTXUI 作为 TUI 库
- 是否使用 nlohmann/json 作为 JSON 库
- 是否使用 yaml-cpp 读取配置和 skill.yaml
- 测试框架选 doctest 还是 Catch2

### 2. Minimal AgentRunner

需要优先落地 AgentRunner，而不是只做宽泛的 AgentLoop。

核心类型：

- [ ] `Message`
- [ ] `ToolCall`
- [ ] `ToolResult`
- [ ] `ProviderResponse`
- [ ] `AgentResult`
- [ ] `SessionEvent`
- [ ] `AgentRunner`
- [ ] `Done` 虚拟工具

下一轮要重点讨论：

- Provider 返回 tool_call 的标准结构
- ToolResult 如何回传给模型
- Session History 如何组织成 messages
- max_loops 默认值和错误处理策略
- 模型普通文本结束和 Done 工具结束如何共存

### 3. Tool System 最小闭环

第一批工具：

- [ ] `list_dir`
- [ ] `read_file`
- [ ] `glob_files`
- [ ] `search_text`
- [ ] `write_file`
- [ ] `edit_file`
- [ ] `run_shell`

下一轮要重点讨论：

- 文件工具如何限制 workspace 根目录
- `edit_file` 使用 old_text/new_text 还是 patch
- `run_shell` 如何处理 timeout、stdout、stderr、exit code
- 工具 schema 是否使用 OpenAI-compatible function calling 格式

### 4. Permission Gate

权限规则：

- [ ] 只读工具自动执行
- [ ] 写文件必须确认
- [ ] 编辑文件必须确认
- [ ] Shell 命令必须确认
- [ ] 用户拒绝后不得执行工具
- [ ] 拒绝结果写入 Session 并回传模型

PermissionGate 不应只支持 yes/no。第一版数据结构应预留：

- [ ] Approve
- [ ] Deny
- [ ] Edit
- [ ] Feedback

下一轮要重点讨论：

- TUI 中如何展示权限确认
- 默认回车是否表示拒绝
- 是否支持本次会话临时允许某类操作
- Edit 后如何更新 tool_call 参数
- Feedback 如何作为 tool_result 回传模型

### 5. MockProvider 与 Evaluation

在真实 Provider 前，先用 MockProvider 验证 AgentRunner：

- [ ] 直接返回文本
- [ ] 返回单个 tool_call
- [ ] 返回多个 tool_call
- [ ] 收到 tool_result 后返回最终答案
- [ ] 返回 Done 工具
- [ ] 模拟 Provider 错误

固定测试集：

- [ ] `test_agent_runner_single_tool_call`
- [ ] `test_agent_runner_multi_tool_call`
- [ ] `test_done_tool_stops_loop`
- [ ] `test_tool_not_found_goes_back_to_model`
- [ ] `test_denied_write_file_goes_back_to_model`
- [ ] `test_denied_run_shell_goes_back_to_model`
- [ ] `test_permission_edit_changes_args`
- [ ] `test_skill_selector_kwoa_cli`
- [ ] `test_kwoa_cli_send_message_requires_confirm`
- [ ] `test_skill_md_cannot_be_exfiltrated`

下一轮要重点讨论：

- MockProvider 脚本化测试用例格式
- 如何写 AgentRunner 单元测试
- 如何证明模型决策 -> 工具调用 -> 结果回传 -> 继续推理的闭环

### 6. Skills Runtime 第一版

第一批 Skills：

- [ ] `repo_reader`
- [ ] `code_editor`
- [ ] `shell_runner`
- [ ] `cpp_project`
- [ ] `tui_agent`
- [ ] `kwoa_cli`

下一轮要重点讨论：

- `skill.yaml` 字段是否足够
- `SKILL.md` 给模型的规则怎么写
- SkillSelector 第一版是否只做关键词匹配
- 没有命中 Skill 时默认加载哪些 Skills
- 是否兼容 kwoa-cli 仓库中的 `skills/kwoa-cli/SKILL.md`
- 如何做 kwoa-cli Skill smoke test

### 7. kwoa-cli Skill 验证

kwoa-cli 不只是被分析项目，而是第一块真实验收石。

验证目标：

- [ ] Agent 能加载 kwoa-cli Skill。
- [ ] Agent 能理解 kwoa-cli 安全规则。
- [ ] Agent 能执行只读命令。
- [ ] Agent 能拦截写命令。
- [ ] Agent 能把工具结果回传模型继续推理。

Smoke test：

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

### 8. TUI 最小界面

第一版 TUI 不做复杂布局，先保证可用：

- [ ] Chat History
- [ ] Status Bar
- [ ] Tool Call Log
- [ ] Permission Panel
- [ ] Input Box
- [ ] `/help`
- [ ] `/clear`
- [ ] `/status`
- [ ] `/model`
- [ ] `/skills`
- [ ] `/exit`

下一轮要重点讨论：

- 输入框和历史消息如何组织
- 工具调用和工具结果是否单独显示
- 权限确认是否阻塞 AgentRunner
- 状态流如何从 AgentRunner 推送到 TUI

## 下一次最建议做的提交

下一次提交建议直接做：

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

目标不是一次写完整 Agent，而是先保证项目可以编译、测试可以跑，并且最小 AgentRunner 闭环可验证。

## 当前不要做

第一阶段先不要做：

- 向量数据库
- 多 Agent
- MCP
- 插件市场
- 自动 Git 提交
- Browser / Computer Use Agent
- 复杂长期记忆
- 复杂上下文压缩
- 完整真实 Provider

先把 C++ 工程骨架、MockProvider、AgentRunner、ToolSystem、PermissionGate、SkillRuntime 跑通。
