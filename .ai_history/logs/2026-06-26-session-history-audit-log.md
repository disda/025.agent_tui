# AI 协作记录：SessionHistory / AuditLog

日期：2026-06-26

## 背景

用户要求继续推进，且每一步先写技术文档再实现。对照作业要求后，下一步确定为：

```text
feat: add session history and audit log
```

原因：作业明确要求用户输入、模型回复、工具调用、工具执行结果、权限拒绝结果和错误信息都进入会话历史。

## 本轮技术文档

新增：

```text
docs/15-session-history-audit-log-design.md
```

文档明确：

- 统一事件结构 `SessionEvent`。
- 内存会话容器 `SessionHistory`。
- JSONL 落盘审计日志 `AuditLog`。
- AgentRunner 记录 user input、assistant message、tool_call、tool_result、permission_denied、user_feedback、error。

## 本轮实现

新增：

```text
include/agent_tui/session/SessionEvent.hpp
include/agent_tui/session/SessionHistory.hpp
include/agent_tui/session/AuditLog.hpp
tests/test_session_history.cpp
```

更新：

```text
include/agent_tui/agent/AgentRunner.hpp
CMakeLists.txt
README.md
TODO.md
docs/16-session-history-audit-log-verification.md
```

## 当前新增能力

`SessionEvent`：

```text
user_input
assistant_message
tool_call
tool_result
permission_denied
user_feedback
error
```

`SessionHistory`：

- 内存保存事件。
- 支持 add / events / clear / empty / size。

`AuditLog`：

- 将事件写入 JSONL。
- 自动创建父目录。
- 支持 append / append_all。

`AgentRunner`：

- 可选接入 `SessionHistory`。
- 记录初始用户输入。
- 记录模型文本回复。
- 记录工具调用和工具结果。
- 记录权限拒绝和用户反馈。
- 记录 provider error、tool not found、max loops exceeded。

## 测试覆盖

新增测试：

```text
test_session_history_records_tool_flow
test_session_history_records_permission_denial
test_audit_log_writes_jsonl
test_json_escape_in_audit_log
```

## 注意

本轮已提交测试代码与 CMake 集成。实际环境可运行：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 下一步建议

下一步进入：

```text
feat: add config loader with user project priority
```

原因：作业测试范围明确要求覆盖配置优先级，配置系统也会被后续 Provider、TUI 和 AgentLoop 使用。
