# SessionHistory / AuditLog 技术设计

## 1. 背景

作业要求明确指出：

```text
用户输入、模型回复、工具调用、工具执行结果、权限拒绝结果和错误信息都应进入会话历史。
```

当前项目已经具备：

```text
AgentRunner
ToolRegistry
MockProvider
FileTools + WorkspaceGuard
PermissionGate
Controlled Shell Tool
Write / Edit Tools
```

下一步需要补齐运行时事件记录能力，让后续 TUI、AgentLoop、Deliverables 和调试分析都有可追踪的数据来源。

## 2. 设计目标

第一版目标：

- 提供统一的 `SessionEvent` 事件结构。
- 提供内存中的 `SessionHistory`。
- 提供写入 JSONL 文件的 `AuditLog`。
- AgentRunner 能记录关键运行事件。
- 支持记录用户输入、模型回复、tool_call、tool_result、permission_denial、user_feedback、error。
- 后续 TUI 可以直接从 SessionHistory 渲染 Chat History 和 Tool Log。
- 后续 deliverables 可以复用 AuditLog 生成运行证明。

## 3. SessionEvent

事件类型：

```cpp
enum class SessionEventType {
    UserInput,
    AssistantMessage,
    ToolCall,
    ToolResult,
    PermissionDenied,
    UserFeedback,
    Error,
};
```

事件字段：

```cpp
struct SessionEvent {
    SessionEventType type;
    std::string content;
    std::string tool_call_id;
    std::string tool_name;
    std::string timestamp;
};
```

第一版不引入 JSON 库，而是手写最小 JSONL 序列化函数：

```cpp
std::string to_json_line() const;
```

## 4. SessionHistory

`SessionHistory` 是内存事件容器：

```cpp
class SessionHistory {
public:
    void add(SessionEvent event);
    const std::vector<SessionEvent>& events() const;
    void clear();
    bool empty() const;
    std::size_t size() const;
};
```

用途：

- TUI 展示当前会话。
- AgentLoop 构造上下文。
- 测试验证工具流是否被记录。
- 未来支持会话清空、导出、压缩。

## 5. AuditLog

`AuditLog` 负责落盘 JSONL：

```cpp
class AuditLog {
public:
    explicit AuditLog(std::filesystem::path path);
    bool append(const SessionEvent& event);
    bool append_all(const SessionHistory& history);
};
```

默认后续写入：

```text
.agent-tui/sessions/<session-id>.jsonl
```

当前第一版只实现指定路径写入，session-id 生成后续由 AgentLoop 负责。

## 6. AgentRunner 集成

AgentRunner 新增可选 `SessionHistory*`：

```cpp
AgentRunner(provider, tools, approval_service, session_history, max_loops)
```

记录规则：

```text
输入 messages 中的 user message -> UserInput
Provider 返回 Text -> AssistantMessage
Provider 返回 Error -> Error
每个 tool_call -> ToolCall
工具执行结果 -> ToolResult
权限拒绝 -> PermissionDenied + ToolResult
用户反馈 -> UserFeedback + ToolResult
Tool not found -> Error + ToolResult
max_loops exceeded -> Error
```

## 7. 测试计划

新增：

```text
tests/test_session_history.cpp
```

覆盖：

- `test_session_history_records_tool_flow`
- `test_session_history_records_permission_denial`
- `test_audit_log_writes_jsonl`

## 8. 非目标

第一版不做：

- 长期记忆。
- 上下文压缩。
- 多会话索引。
- 加密存储。
- 日志轮转。
- UI 展示。

这些后续在 AgentLoop / TUI 阶段补齐。

## 9. 下一步

完成 SessionHistory / AuditLog 后，下一步建议进入：

```text
feat: add config loader with user project priority
```

原因：作业测试范围明确要求覆盖配置优先级。
