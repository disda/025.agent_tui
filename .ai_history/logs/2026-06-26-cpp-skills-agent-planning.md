# AI 协作记录：C++ TUI Agent + Skills Runtime 方案

日期：2026-06-26

## 背景

本项目是 L2 命题任务「从零实现一个 TUI 终端编码 Agent」。原始文档要求实现一个最小可用的本地 TUI Coding Agent，覆盖 Agent Loop、工具系统、权限控制、Provider 接入、会话上下文、配置管理、测试和交付物。

用户明确提出实现方向调整为：

- 使用纯 C++ 实现
- 参考 kwoa-cli 的 skills 思路
- 先将计划整理成文档提交

## 关键讨论结论

### 1. 项目不应继续沿用 Python/Textual 方案

仓库原始实施计划以 Python、Textual、Pydantic、PyYAML、httpx、pytest 为技术栈。用户明确要求纯 C++ 实现，因此实施计划调整为：

- C++20
- CMake
- FTXUI
- nlohmann/json
- yaml-cpp
- cpp-httplib 或 cpr/libcurl
- doctest 或 Catch2

### 2. Agent 应实现为本地运行时，而不是单个 Prompt

确认 Agent 架构为：

```text
Agent = Session + Skill Selector + Agent Loop + Tool Registry + Permission Gate + Provider Adapter
```

运行流程：

```text
用户输入任务
  ↓
TUI 展示状态
  ↓
Skill Selector 选择相关 Skills
  ↓
Agent Loop 请求模型
  ↓
模型返回文本或 tool_call
  ↓
Permission Gate 检查危险操作
  ↓
Tool Registry 执行工具
  ↓
工具结果写入 Session 并回传模型
  ↓
模型继续推理直到完成
```

### 3. 采用 kwoa-cli 风格 Skills Runtime

确认 Skill 与 Tool 的边界：

- Skill：能力描述、触发条件、允许工具集合、给模型的行为规范
- Tool：本地可执行的结构化函数

第一批 Skills：

- `repo_reader`
- `code_editor`
- `shell_runner`
- `cpp_project`
- `tui_agent`

第一批 Tools：

- `list_dir`
- `read_file`
- `glob_files`
- `search_text`
- `write_file`
- `edit_file`
- `run_shell`

### 4. 权限模型

确认权限规则：

- 只读工具自动执行
- 写文件、编辑文件、Shell 命令必须经过用户确认
- 用户拒绝授权时，不得执行工具
- 拒绝结果必须写入 Session，并作为 tool_result 回传模型

### 5. Agent Loop 是核心自研部分

确认 Agent Loop 必须自行实现，不得依赖第三方 Agent SDK 或 Agent Framework。

核心闭环：

```text
模型决策 -> 工具调用 -> 工具执行 -> 结果回传 -> 继续推理
```

Agent Loop 必须支持：

- 多轮 tool_call
- tool not found 错误处理
- permission denied 处理
- provider error 处理
- max_loops 防止无限循环
- Session History 记录所有关键事件

## 已提交文档

本轮协作已提交以下文档更新：

- `README.md`：明确项目方向为纯 C++ TUI Coding Agent + kwoa-cli 风格 Skills Runtime
- `docs/00-implementation-plan.md`：重写为 C++ + Skills Runtime 实施计划
- `docs/01-skill-standard.md`：新增 Skills 标准设计
- `docs/02-agent-loop-and-runtime.md`：新增 Agent Loop 与运行时设计
- `docs/03-delivery-roadmap.md`：新增交付路线与验收清单

相关提交：

- `96a1f8f4` docs: add skill standard
- `b2fc17a0` docs: update project direction
- `47533c91` docs: rewrite implementation plan for C++ skills runtime
- `9e635ae1` docs: add agent loop runtime design
- `1431f34d` docs: add delivery roadmap

## 后续实施建议

下一步建议进入 C++ 工程骨架阶段：

1. 创建 `CMakeLists.txt`
2. 创建 `src/main.cpp`
3. 创建 `include/agent_tui/...`
4. 创建 `tests/...`
5. 实现 MockProvider
6. 实现 SessionHistory 与 AuditLog
7. 实现 Tool 抽象与 ToolRegistry
8. 实现 AgentLoop 最小闭环

## 风险与注意事项

- 不要提前引入向量数据库、多 Agent、MCP、插件市场等扩展能力。
- 第一阶段优先保证 Agent Loop、Tool System、Permission Gate、Session History 和 MockProvider 测试闭环。
- API Key 等敏感信息不得写入仓库或日志。
- AI 协作记录需要持续追加到 `.ai_history/logs/`。
