# Agent Loop 与运行时设计

## 1. 核心目标

Agent Loop 是本项目最核心的自研部分，负责驱动完整的：

```text
模型决策 -> 工具调用 -> 结果回传 -> 继续推理
```

它不能由第三方 Agent SDK 或 Agent Framework 代替。

## 2. 运行流程

```text
User Input
  ↓
Session.add_user
  ↓
SkillSelector.select
  ↓
Build messages + tools schema
  ↓
Provider.chat
  ↓
Parse ProviderResponse
  ↓
If Text: finish
  ↓
If ToolCalls: execute tools
  ↓
ToolResult -> Session
  ↓
Loop again
```

## 3. AgentLoop 伪代码

```cpp
AgentResult AgentLoop::run(const std::string& user_input) {
    session_.add_user(user_input);
    audit_.write_user(user_input);

    auto selected_skills = skill_selector_.select(user_input, session_);
    auto skill_prompt = skill_registry_.build_prompt(selected_skills);
    auto available_tools = tool_registry_.schemas_for_skills(selected_skills);

    for (int step = 0; step < config_.max_loops; ++step) {
        auto messages = session_.build_messages(skill_prompt);

        auto response = provider_.chat(messages, available_tools);

        if (response.type == ProviderResponseType::Text) {
            session_.add_assistant(response.text);
            audit_.write_assistant(response.text);
            return AgentResult::done(response.text);
        }

        if (response.type == ProviderResponseType::ToolCalls) {
            session_.add_tool_calls(response.tool_calls);

            for (const auto& call : response.tool_calls) {
                auto* tool = tool_registry_.find(call.name);

                if (!tool) {
                    session_.add_tool_result(call.id, "Tool not found: " + call.name);
                    continue;
                }

                if (tool->permission_mode() == PermissionMode::Confirm) {
                    auto approved = approval_.request(call);

                    if (!approved) {
                        session_.add_permission_denial(call);
                        session_.add_tool_result(call.id, "User denied permission.");
                        continue;
                    }
                }

                auto result = tool->run(call.arguments);
                session_.add_tool_result(
                    call.id,
                    result.ok ? result.output : result.error
                );

                audit_.write_tool_result(call, result);
            }

            continue;
        }

        if (response.type == ProviderResponseType::Error) {
            session_.add_error(response.error);
            return AgentResult::failed(response.error);
        }
    }

    return AgentResult::failed("Max loop count exceeded.");
}
```

## 4. Tool 执行流程

```text
tool_call
  ↓
ToolRegistry.find
  ↓
PermissionMode check
  ↓
ApprovalService.request
  ↓
Tool.run
  ↓
ToolResult
  ↓
Session.add_tool_result
  ↓
Provider receives tool result
```

## 5. 权限模型

| 操作 | 权限 |
| --- | --- |
| list_dir | Auto |
| read_file | Auto |
| glob_files | Auto |
| search_text | Auto |
| write_file | Confirm |
| edit_file | Confirm |
| run_shell | Confirm |

拒绝授权时：

- 不执行工具
- 记录 `permission_denial`
- 生成 `tool_result: User denied permission.`
- 将拒绝结果回传模型

## 6. Session 事件

Session 需要保存：

- 用户输入
- 模型回复
- 工具调用
- 工具结果
- 权限请求
- 权限拒绝
- Provider 错误
- 工具错误
- 状态变化

## 7. TUI 状态

```text
Idle
Thinking
CallingTool
WaitingApproval
RunningTool
Done
Failed
```

每次状态变化都应推送给 TUI 展示，并进入 Audit Log。

## 8. 最大循环

默认：

```yaml
max_loops: 8
```

超过最大循环后，Agent 停止，并输出明确错误：

```text
Max loop count exceeded.
```

## 9. Provider 边界

Provider 只负责协议适配：

- 发送 messages
- 发送 tools schema
- 解析文本
- 解析 tool_calls
- 处理流式输出
- 超时控制
- 重试

Provider 不负责工具执行，不负责权限确认，不负责会话决策。
