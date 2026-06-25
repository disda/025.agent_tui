# 交付路线与验收清单

## 1. 阶段路线

### 阶段 1：文档与架构确定

交付：

- `docs/00-implementation-plan.md`
- `docs/01-skill-standard.md`
- `docs/02-agent-loop-and-runtime.md`
- `docs/03-delivery-roadmap.md`

目标：

- 明确 C++ 实现路线
- 明确 Skills Runtime 标准
- 明确 Agent Loop 边界
- 明确 L2 交付验收点

### 阶段 2：C++ 工程骨架

交付：

- `CMakeLists.txt`
- `src/main.cpp`
- `include/agent_tui/...`
- `tests/...`

验收：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build
```

### 阶段 3：Session + Tool System

交付：

- SessionHistory
- AuditLog
- Tool 抽象
- ToolRegistry
- FileTools
- ShellTool

验收：

- list_dir 可执行
- read_file 可执行
- search_text 可执行
- write_file 标记为 Confirm
- run_shell 标记为 Confirm

### 阶段 4：Permission Gate

交付：

- ApprovalService
- TUI 权限确认
- 权限拒绝记录

验收：

- 写文件必须确认
- Shell 命令必须确认
- 用户拒绝后工具不得执行
- 拒绝结果进入 Session

### 阶段 5：MockProvider + AgentLoop

交付：

- MockProvider
- AgentLoop
- Tool call parsing
- Tool result 回传

验收：

- MockProvider 能驱动单轮 tool_call
- MockProvider 能驱动多轮 tool_call
- max_loops 生效
- tool not found 能进入上下文

### 阶段 6：Skill Runtime

交付：

- SkillLoader
- SkillRegistry
- SkillSelector
- 默认 skills

验收：

- 能加载 `skills/*/skill.yaml`
- 能读取 `SKILL.md`
- 能按 trigger 选择 skill
- 能限制可用工具集合

### 阶段 7：TUI

交付：

- Chat History
- Status Bar
- Tool Call Log
- Permission Panel
- Input Box
- 内置命令

验收：

- 可输入任务
- 可展示模型回复
- 可展示工具调用
- 可展示工具结果
- 可执行 `/help`、`/clear`、`/status`、`/model`、`/skills`、`/exit`

### 阶段 8：Provider Adapter

交付：

- OpenAICompatibleProvider
- CodingPlanProvider

验收：

- 支持工具 schema
- 支持 tool_call 解析
- 支持流式输出
- 支持超时控制
- 支持基础重试

### 阶段 9：Deliverables

交付：

- `deliverables/README.md`
- `deliverables/demo-task.md`
- 运行截图
- AI 协作记录 `.ai_history/logs/`

验收任务示例：

```text
请读取当前项目 README，并新增一节 Build，写入 CMake 构建命令，然后运行测试。
```

Agent 应完成：

1. read_file README.md
2. write_file README.md，需要用户确认
3. run_shell cmake -S . -B build，需要用户确认
4. run_shell cmake --build build，需要用户确认
5. 输出最终结果

## 2. 必须测试项

- Agent 主循环
- 工具调用与结果回传
- 权限确认
- 权限拒绝
- 配置优先级
- Mock Provider
- Skill 加载
- Skill 选择
- Tool Registry
- 文件路径逃逸防护
- max_loops

## 3. 不要提前做的功能

第一阶段不要做：

- 向量数据库
- 多 Agent
- 插件市场
- MCP
- 复杂长期记忆
- 自动 Git 提交
- 复杂 UI 布局

先保证最小闭环稳定。

## 4. 最小完成定义

项目达到以下状态即可进入第一轮验收：

```text
TUI 可运行
用户可输入任务
AgentLoop 可调用 MockProvider
MockProvider 可产生 tool_call
ToolRegistry 可执行工具
PermissionGate 可拦截危险工具
工具结果可回传模型
Session 可记录完整历史
AuditLog 可写入 .ai_history/logs
测试覆盖核心路径
```
