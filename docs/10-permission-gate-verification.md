# 构建验证记录：PermissionGate

日期：2026-06-26

## 1. 验证目标

本次验证目标是实现 AgentRunner 中的权限确认机制，为后续 `run_shell`、`write_file`、`edit_file`、`run_kwoa_cli` 提供安全拦截能力。

新增内容：

```text
include/agent_tui/permissions/Approval.hpp
include/agent_tui/permissions/ApprovalService.hpp
include/agent_tui/permissions/MockApprovalService.hpp
tests/test_permission_gate.cpp
```

更新：

```text
include/agent_tui/agent/AgentRunner.hpp
CMakeLists.txt
```

## 2. 新增能力

### ApprovalType

```cpp
enum class ApprovalType {
    Approve,
    Deny,
    Edit,
    Feedback,
};
```

### ApprovalDecision

支持四类决策：

```text
Approve: 按原始参数执行。
Deny: 不执行，回传 User denied permission。
Edit: 使用用户编辑后的参数执行。
Feedback: 不执行，把用户反馈回传模型。
```

### ApprovalService

`ApprovalService` 是权限交互抽象：

```cpp
class ApprovalService {
public:
    virtual ~ApprovalService() = default;
    virtual ApprovalDecision request(const ToolCall& call, const Tool& tool) = 0;
};
```

当前提供测试实现：

```text
MockApprovalService
```

后续 TUI 将提供真实实现：

```text
TuiApprovalService
```

## 3. AgentRunner 集成

`AgentRunner` 现在在执行工具前检查：

```text
PermissionMode::Auto -> 直接执行
PermissionMode::Confirm -> 请求 ApprovalService
```

Confirm 工具处理规则：

```text
Approve -> 执行原始参数
Deny -> 不执行，tool_result = User denied permission.
Edit -> 使用 edited_arguments 执行
Feedback -> 不执行，tool_result = User feedback: ...
```

如果没有配置 ApprovalService：

```text
Permission required but no approval service configured.
```

工具不会被执行。

## 4. 测试覆盖

`tests/test_permission_gate.cpp` 覆盖：

- Confirm 工具 Deny 后不执行。
- Deny 结果作为 tool_result 回传模型。
- Confirm 工具 Approve 后执行。
- Confirm 工具 Edit 后使用编辑后的参数执行。
- Confirm 工具 Feedback 后不执行，并把反馈回传模型。

## 5. 本地隔离构建环境

验证命令：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 6. 验证结果

本地沙箱验证通过：

```text
[100%] Built target agent_tui_permission_gate_tests
Test project /mnt/data/agent_tui_perm/build
    Start 1: agent_tui_tests
1/3 Test #1: agent_tui_tests ...................   Passed    0.02 sec
    Start 2: agent_tui_file_tools_tests
2/3 Test #2: agent_tui_file_tools_tests ........   Passed    0.02 sec
    Start 3: agent_tui_permission_gate_tests
3/3 Test #3: agent_tui_permission_gate_tests ...   Passed    0.02 sec

100% tests passed, 0 tests failed out of 3
```

## 7. 下一步

下一步建议实现：

```text
feat: add controlled shell tool
```

原因：`run_shell` 是高风险工具，现在已有 PermissionGate，可以进入受控执行能力。
