# Demo：加载 kwoa-cli Skill 的真实验证任务

## 1. Demo 目标

本 Demo 用于证明 `agent_tui` 不是普通聊天程序，而是一个能加载真实 Skill、调用工具、执行权限确认、根据工具结果继续推理的本地 Coding Agent Harness。

验证对象：

```text
kwoa-cli Skill
```

验证能力：

```text
IM 最近会话读取
IM 消息搜索/上下文读取
KDocs 文档读取
KDocs 文档写入 dry-run / confirmed write
消息发送/撤回/转发/反应的权限拦截
```

## 2. 前置条件

### 2.1 agent_tui 已构建

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### 2.2 kwoa-cli 可用

Agent 应按以下顺序查找 kwoa-cli：

```text
1. 已安装的 kwoa-cli skill 包中的 bin/kwoa-cli
2. 用户配置的 kwoa-cli 路径
3. 本地 workspace 中的 kwoa-cli 仓库构建产物
4. 经用户确认后 clone / build
```

### 2.3 kwoa-cli 认证状态

只读 live WOA 数据前先运行：

```bash
kwoa-cli auth status --format pretty
```

如果未初始化，Agent 应提示用户更新 `.env` 中的 `KWOA_COOKIE`，并停止任务，不猜测数据。

## 3. Demo 任务 A：读取最近会话

用户输入：

```text
用 kwoa-cli 看看最近会话，不要发送消息。
```

期望流程：

```text
User Input
  ↓
SkillSelector 命中 kwoa_cli
  ↓
加载 kwoa-cli Skill 规则
  ↓
run_kwoa_cli: auth status
  ↓
如果 initialized=false：提示用户更新 cookie
如果 initialized=true：继续 recent-list
  ↓
run_kwoa_cli: im +recent-list --count 20 --compact --format json
  ↓
ToolResult 回传模型
  ↓
模型总结最近会话，并说明命令、限制、数据来源
```

禁止行为：

- 不得发送消息。
- 不得打印 `.env` 或 cookie。
- 不得无限分页。
- 不得在认证失败时编造数据。

## 4. Demo 任务 B：读取 KDocs 文档

用户输入：

```text
读取这个 KDocs 文档并总结内容：<file_id 或 link_id>，不要写入。
```

期望流程：

```text
SkillSelector 命中 kwoa_cli
  ↓
run_kwoa_cli: docs +info
  ↓
run_kwoa_cli: docs +read --content-format markdown --include-elements all --output <temp.md>
  ↓
read_file <temp.md>
  ↓
模型总结文档内容，说明读取命令和限制
```

禁止行为：

- 不得执行 `docs +markdown-insert --yes`。
- 不得把 KDocs token 写入 `.env`。
- 不得隐藏读取失败原因。

## 5. Demo 任务 C：写操作权限拦截

用户输入：

```text
给 chat 123 发送 hello。
```

期望流程：

```text
Agent 识别 messages-send 是高风险写操作
  ↓
展示即将执行的命令
  ↓
PermissionGate 请求确认
  ↓
用户拒绝：不执行，把 User denied permission 作为 tool_result 回传模型
用户确认：执行带 --yes 的真实写操作
```

必须拦截的写操作：

```text
im +messages-send --yes
im +messages-recall --yes
im +messages-forward --yes
im +reaction-send --yes
docs +markdown-insert --yes
```

## 6. 验收截图清单

最终交付时，`deliverables/screenshots/` 应包含：

```text
01-tui-start.png
02-kwoa-skill-selected.png
03-auth-status-tool-call.png
04-recent-list-tool-result.png
05-permission-panel-deny.png
06-final-answer.png
```

## 7. 当前状态

当前仓库已完成：

- C++ 工程骨架
- Minimal AgentRunner
- MockProvider
- ToolRegistry 抽象
- Done 虚拟工具处理
- 基础 AgentRunner 单元测试

尚未完成：

- kwoa_cli Skill 加载
- run_kwoa_cli 工具
- PermissionGate
- TUI 截图
