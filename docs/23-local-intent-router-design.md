# Local Intent Router 技术设计

## 1. 背景

当前 `agent_tui` 已能通过 OpenAI-compatible Provider 做真实文本对话，但真实工具调用仍未接入：

```text
OpenAI-compatible tool_calls 解析
Tools schema 转换
AgentLoop 应用层
```

因此用户在 TUI 中输入很多自然语言任务时，模型只能回复文本，不能真正执行本地工具。

为了解决“很多都执行不了”的问题，本轮先加入一个轻量级本地 Intent Router：在进入 Provider 之前，优先识别常见本地开发意图，并直接调用已有工具。

## 2. 设计目标

第一版目标：

- 支持常见本地开发命令的规则识别。
- 支持中英文关键词。
- 支持直接调用本地工具。
- 支持 shell 类命令确认后执行。
- Windows 下 `run_shell` 不再直接 unsupported，先提供 `_popen` 简版。
- 未识别意图时仍走 Provider 对话。

## 3. 支持意图

### ListDir

示例：

```text
ls
dir
list
列出目录
看看目录
```

调用：

```text
list_dir
```

### ReadFile

示例：

```text
read README.md
cat README.md
读取 README.md
打开 README.md
```

调用：

```text
read_file
```

### SearchText

示例：

```text
search AgentRunner
grep AgentRunner
搜索 AgentRunner
查找 AgentRunner
```

调用：

```text
search_text
```

### ConfigureProject

示例：

```text
configure
配置 cmake
生成 build
cmake configure
```

调用：

```text
run_shell: cmake -S . -B build
```

### BuildProject

示例：

```text
build
编译
构建项目
```

调用：

```text
run_shell: cmake --build build
```

### TestProject

示例：

```text
test
ctest
运行测试
执行测试
```

调用：

```text
run_shell: ctest --test-dir build --output-on-failure
```

## 4. 安全策略

只读意图自动执行：

```text
list_dir
read_file
search_text
```

Shell 意图必须确认：

```text
configure
build
test
```

TUI 会提示：

```text
Approve run_shell: <command> ? [y/N]
```

用户拒绝时：

- 不执行命令。
- 记录 `permission_denied`。
- 记录 `tool_result = User denied permission.`。

## 5. 与 Provider 的关系

执行顺序：

```text
用户输入
  ↓
LocalIntentRouter 尝试识别
  ↓
如果命中本地意图：执行本地工具
如果未命中：走 Provider 文本对话 / 未来 tool_calls
```

本地路由不是替代模型 tool_calls，而是在 tool_calls 完成前，提高本地 Agent 可用性。

## 6. 后续演进

后续应继续实现：

```text
OpenAI-compatible tool_calls 解析
ToolSchema 导出
AgentLoop 状态机
SkillRuntime
```

届时 LocalIntentRouter 可以继续保留为快捷命令层，也可以退化为 SkillSelector 的 fallback。
