# TUI 终端编码 Agent 中文实施计划

> **给 Agent 执行者：** 实施本计划时，建议按任务逐项推进。每个任务都应先写测试，再实现，再运行验证。若使用 Superpowers 工作流，实施阶段使用 `superpowers:subagent-driven-development` 或 `superpowers:executing-plans`。

**目标：** 从零实现一个最小可用的本地 TUI 编码 Agent，使其能理解用户开发任务、读取仓库上下文、调用结构化工具、执行权限确认、把工具结果回传给模型继续推理，并产出 L2 认证要求的测试与交付物。

**总体架构：** 使用 Python 实现一个小型应用。TUI 负责输入、输出、状态展示和权限确认；Agent Loop 负责模型决策、工具调用、结果回传和循环终止；工具系统负责仓库读写和命令执行；Provider 层负责对接 Mock Provider 与 WPS CodingPlan 协议。核心 Agent Loop、工具系统、权限控制和会话管理全部由项目自行实现，不依赖第三方 Agent SDK 或 Agent Framework。

**技术栈：** Python 3.11+、Textual、Pydantic、PyYAML、httpx、pytest、pytest-asyncio、ruff、mypy。

---

## 一、需求拆解

源需求位于：[题目 Markdown 原文](../output/l2-agent-tui-task.md)。

硬性验收点如下：

- TUI 必须展示用户输入、模型回复、工具调用过程、工具执行结果、权限确认信息和当前运行状态。
- Agent 必须实现完整的「模型决策 -> 工具调用 -> 结果回传 -> 继续推理」流程。
- 仓库工具至少包括目录浏览、文件读取、文件匹配、内容搜索、文件写入或编辑、Shell 命令执行。
- 只读工具可以自动执行；文件写入、文件编辑、Shell 命令执行必须经过用户确认。
- 用户拒绝授权时，对应操作不得执行，拒绝结果必须进入会话上下文。
- 需要支持 WPS CodingPlan 协议，包括工具调用解析、流式输出、超时控制和基础重试。
- 会话历史必须记录用户输入、模型回复、工具调用、工具结果、权限拒绝和错误信息。
- 配置需要支持用户级和项目级，且项目级配置优先于用户级配置。
- TUI 需要提供内置命令：帮助、清空会话、查看或切换模型、查看状态、退出。
- 关键 AI 协作内容需要写入 `.ai_history/logs/`。
- 交付物需要包含完整源码、测试、AI 协作记录和 `deliverables/` 下的可运行验证产物。

## 二、目录规划

```text
.
├── README.md
├── pyproject.toml
├── docs/
│   └── 00-implementation-plan.md
├── output/
│   └── l2-agent-tui-task.md
├── src/
│   └── agent_tui/
│       ├── __init__.py
│       ├── __main__.py
│       ├── app.py
│       ├── config.py
│       ├── logging.py
│       ├── permissions.py
│       ├── agent/
│       │   ├── events.py
│       │   ├── loop.py
│       │   └── session.py
│       ├── llm/
│       │   ├── base.py
│       │   ├── codingplan.py
│       │   └── mock.py
│       └── tools/
│           ├── base.py
│           ├── filesystem.py
│           └── shell.py
├── tests/
│   ├── test_agent_loop.py
│   ├── test_codingplan_provider.py
│   ├── test_commands.py
│   ├── test_config.py
│   ├── test_permissions.py
│   ├── test_session.py
│   └── test_tools.py
├── .ai_history/
│   └── logs/
│       └── .gitkeep
└── deliverables/
    ├── README.md
    └── demo-task.md
```

## 三、模块职责

- `src/agent_tui/app.py`：Textual TUI 应用，负责消息流、输入框、状态栏、权限弹窗和内置命令。
- `src/agent_tui/agent/loop.py`：自研 Agent Loop，负责模型调用、工具执行、结果回传、循环终止和错误处理。
- `src/agent_tui/agent/session.py`：会话历史管理，记录用户消息、助手消息、工具调用、工具结果、拒绝授权和错误。
- `src/agent_tui/agent/events.py`：定义历史事件与运行时事件的数据结构。
- `src/agent_tui/config.py`：读取用户级配置和项目级配置，完成优先级合并与敏感字段脱敏。
- `src/agent_tui/llm/base.py`：Provider 协议、流式事件和工具调用结构。
- `src/agent_tui/llm/mock.py`：测试与演示用 Mock Provider。
- `src/agent_tui/llm/codingplan.py`：WPS CodingPlan 协议适配，支持流式解析、工具调用解析、超时与重试。
- `src/agent_tui/tools/base.py`：工具接口、工具结果、权限模式和工具注册表。
- `src/agent_tui/tools/filesystem.py`：目录浏览、文件读取、文件匹配、内容搜索、文件写入、文件编辑。
- `src/agent_tui/tools/shell.py`：Shell 命令执行，支持超时、返回码、标准输出和标准错误。
- `src/agent_tui/permissions.py`：权限请求、用户决策和拒绝记录。
- `src/agent_tui/logging.py`：把关键 AI 协作记录写入 `.ai_history/logs/session.jsonl`。

## 四、实施任务

### 任务 1：初始化项目骨架

**目标：** 建立可安装、可测试、可运行的 Python 包结构。

**创建文件：**

- `pyproject.toml`
- `.gitignore`
- `src/agent_tui/__init__.py`
- `src/agent_tui/__main__.py`

**步骤：**

- [ ] 创建 `pyproject.toml`，声明包名 `agent-tui`、Python 版本、运行依赖和开发依赖。
- [ ] 配置命令行入口：`agent-tui = "agent_tui.__main__:main"`。
- [ ] 创建 `.gitignore`，排除 `.venv/`、缓存目录、`.env`、本地私密配置。
- [ ] 创建 `src/agent_tui/__init__.py`，声明版本号。
- [ ] 创建 `src/agent_tui/__main__.py`，从 `agent_tui.app` 启动 TUI。
- [ ] 运行 `python -m compileall src`，确认包结构可编译。

**验收：**

- `pip install -e ".[dev]"` 可安装项目。
- `agent-tui` 命令可以找到入口。
- `python -m compileall src` 通过。

### 任务 2：实现会话历史与审计日志

**目标：** 所有关键交互都能进入会话历史，并可沉淀到 `.ai_history/logs/`。

**创建文件：**

- `src/agent_tui/agent/events.py`
- `src/agent_tui/agent/session.py`
- `src/agent_tui/logging.py`
- `tests/test_session.py`

**步骤：**

- [ ] 在 `events.py` 中定义 `HistoryItem`，字段包括 `kind`、`payload`、`created_at`。
- [ ] 支持事件类型：`user`、`assistant`、`tool_call`、`tool_result`、`permission_request`、`permission_denial`、`error`、`status`。
- [ ] 在 `session.py` 中实现 `SessionHistory`。
- [ ] 提供 `add_user()`、`add_assistant()`、`add_tool_call()`、`add_tool_result()`、`add_permission_denial()`、`add_error()`、`clear()`。
- [ ] 在 `logging.py` 中实现 `AuditLog`，把 `HistoryItem` 追加写入 `.ai_history/logs/session.jsonl`。
- [ ] 编写测试，验证各种事件都会按顺序进入历史。

**验收：**

- `pytest tests/test_session.py -q` 通过。
- 拒绝授权和错误信息有明确事件类型。

### 任务 3：实现配置加载与脱敏

**目标：** 支持用户级配置和项目级配置，项目配置优先，敏感信息不得裸露。

**创建文件：**

- `src/agent_tui/config.py`
- `tests/test_config.py`

**配置字段：**

- `provider`
- `model`
- `api_base`
- `api_key`
- `timeout_seconds`
- `max_loops`

**步骤：**

- [ ] 使用 Pydantic 定义 `AppConfig`。
- [ ] 默认读取用户级配置 `~/.agent-tui.yaml`。
- [ ] 默认读取项目级配置 `./.agent-tui.yaml`。
- [ ] 合并顺序为：默认值 < 用户级配置 < 项目级配置 < 环境变量中的 API Key。
- [ ] 实现 `redact_config()`，把 `api_key` 显示为 `***`。
- [ ] 编写测试验证项目配置覆盖用户配置。
- [ ] 编写测试验证 API Key 脱敏。

**验收：**

- `pytest tests/test_config.py -q` 通过。
- TUI 状态展示中不能出现真实 API Key。

### 任务 4：实现工具系统与仓库工具

**目标：** 提供结构化工具注册、工具 Schema、工具执行结果和权限模式。

**创建文件：**

- `src/agent_tui/tools/base.py`
- `src/agent_tui/tools/filesystem.py`
- `src/agent_tui/tools/shell.py`
- `tests/test_tools.py`

**工具清单：**

- `list_dir`：浏览目录，只读，自动执行。
- `read_file`：读取文件，只读，自动执行。
- `glob_files`：文件匹配，只读，自动执行。
- `search_text`：内容搜索，只读，自动执行。
- `write_file`：写入文件，必须确认。
- `edit_file`：替换编辑文件，必须确认。
- `run_shell`：执行 Shell 命令，必须确认。

**步骤：**

- [ ] 在 `base.py` 中定义 `Tool`、`ToolResult`、`ToolRegistry` 和权限模式 `auto/confirm`。
- [ ] 每个工具都要暴露 `name`、`description`、`parameters`、`permission` 和 `run()`。
- [ ] 文件工具必须限制在项目根目录内，禁止通过 `../` 逃逸。
- [ ] `search_text` 优先使用 Python 标准库遍历文本文件，保证跨平台可测。
- [ ] `write_file` 自动创建父目录，但执行前由 Agent Loop 请求确认。
- [ ] `edit_file` 使用明确的 `old_text` 和 `new_text`，找不到旧文本时返回失败。
- [ ] Shell 工具使用 `subprocess.run()`，设置 `cwd`、`timeout`、`capture_output=True`、`text=True`。
- [ ] 编写测试验证只读工具权限为 `auto`，写入和 Shell 工具权限为 `confirm`。

**验收：**

- `pytest tests/test_tools.py -q` 通过。
- 所有工具失败时返回结构化错误，不直接抛出未处理异常。

### 任务 5：实现权限确认

**目标：** 写文件、编辑文件和 Shell 命令必须经过用户授权；拒绝结果必须进入会话历史。

**创建文件：**

- `src/agent_tui/permissions.py`
- `tests/test_permissions.py`

**步骤：**

- [ ] 定义 `PermissionRequest`，包含工具名和参数。
- [ ] 定义 `PermissionDecision`，包含 `approved` 和 `reason`。
- [ ] 实现 `PermissionGate`，通过回调函数向 TUI 请求确认。
- [ ] 用户拒绝时调用 `SessionHistory.add_permission_denial()`。
- [ ] 编写测试，确认拒绝后工具不会执行，历史中有拒绝记录。

**验收：**

- `pytest tests/test_permissions.py -q` 通过。
- 拒绝授权时，模型下一轮能看到拒绝结果。

### 任务 6：实现 Provider 抽象和 WPS CodingPlan 适配

**目标：** Provider 层支持 Mock 测试，也支持 WPS CodingPlan 协议的流式调用与工具调用解析。

**创建文件：**

- `src/agent_tui/llm/base.py`
- `src/agent_tui/llm/mock.py`
- `src/agent_tui/llm/codingplan.py`
- `tests/test_codingplan_provider.py`

**步骤：**

- [ ] 在 `base.py` 中定义 `ProviderRequest`、`StreamEvent`、`ToolCall` 和 `LlmProvider` 协议。
- [ ] `StreamEvent` 支持 `text`、`tool_call`、`done`、`error`。
- [ ] 在 `mock.py` 中实现可注入事件序列的 `MockProvider`。
- [ ] 在 `codingplan.py` 中实现 `parse_codingplan_line()`。
- [ ] 支持解析文本增量事件。
- [ ] 支持解析工具调用事件，拿到工具名和 JSON 参数。
- [ ] 支持 `[DONE]` 或等价结束事件。
- [ ] 使用 `httpx.AsyncClient` 实现流式请求。
- [ ] 使用配置中的 `timeout_seconds` 控制超时。
- [ ] 对网络错误或超时做 1 次基础重试。
- [ ] 编写测试覆盖文本增量、工具调用、结束事件和错误事件。

**验收：**

- `pytest tests/test_codingplan_provider.py -q` 通过。
- Provider 层不包含 Agent Loop 逻辑。

### 任务 7：实现 Agent Loop

**目标：** 串起模型、工具、权限、会话历史和循环控制。

**创建文件：**

- `src/agent_tui/agent/loop.py`
- `tests/test_agent_loop.py`

**循环流程：**

```text
用户输入
  -> 写入会话历史
  -> 调用 Provider.stream()
  -> 展示模型流式输出
  -> 若收到 tool_call
      -> 记录工具调用
      -> 判断工具权限
      -> 自动执行或请求确认
      -> 记录工具结果或拒绝结果
      -> 把结果加入下一轮模型上下文
  -> 直到 done、error 或 max_loops 达到上限
```

**步骤：**

- [ ] 实现 `AgentLoop.run(user_input)` 异步生成器。
- [ ] 把 `SessionHistory` 转成 Provider 消息列表。
- [ ] 把 `ToolRegistry.schemas()` 传给 Provider。
- [ ] 收到 `text` 事件时，向 TUI 产出助手文本并记录历史。
- [ ] 收到 `tool_call` 事件时，查找工具并记录调用。
- [ ] 对 `permission="auto"` 的工具直接执行。
- [ ] 对 `permission="confirm"` 的工具调用 `PermissionGate`。
- [ ] 用户拒绝时，不执行工具，把拒绝记录加入历史。
- [ ] 工具执行成功或失败都写入 `tool_result`。
- [ ] 达到 `max_loops` 时停止，并记录状态事件。
- [ ] 编写测试覆盖工具结果回传、拒绝授权不执行工具、错误进入历史。

**验收：**

- `pytest tests/test_agent_loop.py -q` 通过。
- Agent Loop 不依赖任何第三方 Agent 框架。

### 任务 8：实现 TUI 和内置命令

**目标：** 用户可以在终端界面中输入任务、观察 Agent 状态、确认权限、执行内置命令。

**创建文件：**

- `src/agent_tui/app.py`
- `tests/test_commands.py`

**界面区域：**

- 消息流：展示用户输入、助手回复、工具调用和工具结果。
- 输入框：接收自然语言任务和 `/` 命令。
- 状态栏：展示 Provider、模型、循环状态、当前工具和权限等待状态。
- 权限确认弹窗：展示工具名、参数和影响，提供批准/拒绝。

**内置命令：**

- `/help`：展示命令帮助。
- `/clear`：清空当前会话。
- `/model`：查看当前模型。
- `/model <name>`：切换当前模型。
- `/status`：查看配置、Provider、模型、循环状态，敏感信息必须脱敏。
- `/exit`：退出程序。

**步骤：**

- [ ] 实现 `parse_builtin_command()`，把输入解析为命令名和参数。
- [ ] 为命令解析编写单元测试。
- [ ] 使用 Textual 创建主应用。
- [ ] 接入 `AgentLoop.run()`，把运行事件渲染到消息流。
- [ ] 实现权限确认弹窗，并把用户选择传回 `PermissionGate`。
- [ ] 实现状态栏更新。
- [ ] 手动运行 `agent-tui`，验证 `/help`、`/status`、`/model`、`/clear`、`/exit`。

**验收：**

- `pytest tests/test_commands.py -q` 通过。
- 终端中能看到工具调用、工具结果和权限确认。

### 任务 9：补齐交付物和验证材料

**目标：** 满足 L2 提交要求。

**创建文件：**

- `.ai_history/logs/.gitkeep`
- `deliverables/README.md`
- `deliverables/demo-task.md`

**步骤：**

- [ ] 创建 `.ai_history/logs/.gitkeep`，保证日志目录进入仓库。
- [ ] 创建 `deliverables/demo-task.md`，写入一个可由 Agent 完成的小任务。
- [ ] 创建 `deliverables/README.md`，说明需要放置的验证截图或运行记录。
- [ ] 用 Agent 自己完成一个小任务，例如创建并运行一个文本小游戏或小脚本。
- [ ] 把关键截图或运行记录放入 `deliverables/`。
- [ ] 更新根目录 `README.md`，补充运行方式、测试方式、配置方式和 L2 勾选清单。

**验收：**

- `deliverables/` 中有可说明 Agent 能运行的验证材料。
- `.ai_history/logs/` 中有关键 AI 协作记录。

## 五、测试计划

必须覆盖：

- Agent 主循环。
- 工具调用与工具结果回传。
- 权限确认与权限拒绝。
- 配置优先级。
- Mock LLM Provider 场景。
- WPS CodingPlan 流式事件解析。
- 文件工具和 Shell 工具。
- TUI 内置命令解析。

最终验证命令：

```bash
pytest
ruff check .
mypy src
python -m compileall src
```

## 六、里程碑

### 里程碑 1：基础工程可运行

完成任务 1、2、3。

产出：

- 包结构完整。
- 配置系统可用。
- 会话历史和审计日志可用。

### 里程碑 2：Agent Core 可测试

完成任务 4、5、6、7。

产出：

- 工具注册和执行可用。
- 权限控制可用。
- Mock Provider 和 CodingPlan Provider 可用。
- Agent Loop 能根据工具结果继续推理。

### 里程碑 3：TUI 可交互

完成任务 8。

产出：

- 终端界面可运行。
- 用户能输入任务。
- 用户能看到状态、工具过程和权限弹窗。

### 里程碑 4：提交材料完整

完成任务 9。

产出：

- 测试通过。
- `.ai_history/logs/` 有记录。
- `deliverables/` 有验证材料。
- README 具备运行、测试、配置和验收说明。

## 七、风险与处理

- **CodingPlan 事件格式可能与假设不完全一致。** 处理方式：把协议解析集中在 `parse_codingplan_line()`，拿到真实返回后只改适配层，不影响 Agent Loop。
- **TUI 测试容易脆弱。** 处理方式：把命令解析、权限决策、Agent Loop 放在 UI 外部，优先用单元测试覆盖核心逻辑。
- **Shell 命令有破坏性风险。** 处理方式：所有 Shell 命令必须确认，执行目录固定在项目根目录，输出和错误结构化记录。
- **敏感信息泄漏风险。** 处理方式：API Key 只从配置或环境变量读取，状态展示和日志中统一脱敏。
- **文件路径逃逸风险。** 处理方式：所有文件工具使用 `resolve()` 后校验路径必须位于项目根目录内。

## 八、当前文档状态

- 根目录 `README.md` 是项目入口。
- 本文件是中文实施计划。
- 原始题目 Markdown 保留在 `output/l2-agent-tui-task.md`。
