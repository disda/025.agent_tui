# 构建验证记录：Write / Edit Tools

日期：2026-06-26

## 1. 验证目标

本次验证目标是实现真实文件修改工具：

```text
write_file
edit_file
```

这两个工具让 Agent 具备修改项目源码和文档的能力，是 Coding Agent 完成真实开发任务的必要能力。

## 2. 新增内容

新增：

```text
include/agent_tui/tools/WriteEditTools.hpp
tests/test_write_edit_tools.cpp
```

更新：

```text
CMakeLists.txt
```

## 3. 当前能力

### write_file

工具名：

```text
write_file
```

权限：

```text
PermissionMode::Confirm
```

参数：

```text
path
content
create_parent_dirs
```

能力：

- 写入 workspace 内文件。
- 可选创建父目录。
- 拒绝 workspace 外路径。
- 拒绝将目录作为文件写入。
- 返回写入路径和字节数。

### edit_file

工具名：

```text
edit_file
```

权限：

```text
PermissionMode::Confirm
```

参数：

```text
path
old_text
new_text
replace_all
```

能力：

- 读取 workspace 内文本文件。
- 查找 `old_text`。
- 默认只替换第一次匹配。
- `replace_all=true` 时替换全部匹配。
- 未找到 `old_text` 时失败，不写回。
- 返回编辑路径和替换次数。

## 4. 安全边界

`write_file` 和 `edit_file` 都是高风险工具：

```text
PermissionMode::Confirm
```

因此必须经过 AgentRunner 的 PermissionGate。

如果用户拒绝：

```text
不执行工具
不落盘
tool_result = User denied permission.
```

所有路径都通过：

```text
Workspace.resolve(path)
```

因此会拒绝：

```text
../escape.txt
/etc/passwd
../../other-project/file
```

## 5. 测试覆盖

`tests/test_write_edit_tools.cpp` 覆盖：

- `test_write_file_approved_creates_file`
- `test_write_file_denied_not_created`
- `test_write_file_rejects_path_escape`
- `test_edit_file_approved_replaces_text`
- `test_edit_file_denied_not_modified`
- `test_edit_file_missing_old_text_fails`
- `test_edit_file_replace_all`

## 6. 本地隔离构建环境

验证命令：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 7. 验证结果

本地沙箱验证通过：

```text
Test project /mnt/data/agent_tui_validate/build
    Start 1: agent_tui_tests
1/5 Test #1: agent_tui_tests ....................   Passed    0.01 sec
    Start 2: agent_tui_file_tools_tests
2/5 Test #2: agent_tui_file_tools_tests .........   Passed    0.01 sec
    Start 3: agent_tui_permission_gate_tests
3/5 Test #3: agent_tui_permission_gate_tests ....   Passed    0.01 sec
    Start 4: agent_tui_shell_tool_tests
4/5 Test #4: agent_tui_shell_tool_tests .........   Passed    2.05 sec
    Start 5: agent_tui_write_edit_tools_tests
5/5 Test #5: agent_tui_write_edit_tools_tests ...   Passed    0.02 sec

100% tests passed, 0 tests failed out of 5
```

## 8. 当前限制

第一版限制：

- `edit_file` 只支持精确文本替换。
- 不支持 diff/patch 格式。
- 不支持 AST 级编辑。
- 不支持多文件事务。
- 不自动格式化。
- 不自动 Git commit。

## 9. 下一步

下一步建议进入：

```text
feat: add session history and audit log
```

原因：当前已经具备读文件、搜索代码、写文件、编辑文件、执行受控 shell、权限确认与工具结果回传能力。接下来应按交付要求记录用户输入、模型回复、工具调用、工具结果、权限拒绝和错误信息。
