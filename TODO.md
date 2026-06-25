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
- [x] 按项目要求补充 `.ai_history/logs/` AI 协作记录。

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

### 2. 最小 Agent Loop 数据结构

需要先落地核心类型：

- [ ] `Message`
- [ ] `ToolCall`
- [ ] `ToolResult`
- [ ] `ProviderResponse`
- [ ] `AgentResult`
- [ ] `SessionEvent`

下一轮要重点讨论：

- Provider 返回 tool_call 的标准结构
- ToolResult 如何回传给模型
- Session History 如何组织成 messages
- max_loops 默认值和错误处理策略

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

下一轮要重点讨论：

- TUI 中如何展示权限确认
- 默认回车是否表示拒绝
- 是否支持本次会话临时允许某类操作

### 5. MockProvider

在真实 Provider 前，先用 MockProvider 验证 Agent Loop：

- [ ] 直接返回文本
- [ ] 返回单个 tool_call
- [ ] 返回多个 tool_call
- [ ] 收到 tool_result 后返回最终答案
- [ ] 模拟 Provider 错误

下一轮要重点讨论：

- MockProvider 脚本化测试用例格式
- 如何写 AgentLoop 单元测试
- 如何证明模型决策 -> 工具调用 -> 结果回传 -> 继续推理的闭环

### 6. Skills Runtime 第一版

第一批 Skills：

- [ ] `repo_reader`
- [ ] `code_editor`
- [ ] `shell_runner`
- [ ] `cpp_project`
- [ ] `tui_agent`

下一轮要重点讨论：

- `skill.yaml` 字段是否足够
- `SKILL.md` 给模型的规则怎么写
- SkillSelector 第一版是否只做关键词匹配
- 没有命中 Skill 时默认加载哪些 Skills

### 7. TUI 最小界面

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
- 权限确认是否阻塞 Agent Loop
- 状态流如何从 AgentLoop 推送到 TUI

## 下一次最建议做的提交

下一次提交建议直接做：

```text
feat: add initial C++ project skeleton
```

包含：

- `CMakeLists.txt`
- `src/main.cpp`
- `include/agent_tui/agent/Message.hpp`
- `include/agent_tui/agent/ToolCall.hpp`
- `include/agent_tui/tools/Tool.hpp`
- `include/agent_tui/llm/Provider.hpp`
- `tests/test_placeholder.cpp`

目标不是一次写完整 Agent，而是先保证项目可以编译、测试可以跑。

## 当前不要做

第一阶段先不要做：

- 向量数据库
- 多 Agent
- MCP
- 插件市场
- 自动 Git 提交
- 复杂长期记忆
- 复杂上下文压缩
- 完整真实 Provider

先把 C++ 工程骨架、MockProvider、AgentLoop、ToolSystem、PermissionGate 跑通。
