# Write / Edit Tools 技术设计

## 1. 背景

`agent_tui` 已完成：

```text
Minimal AgentRunner
FileTools + WorkspaceGuard
PermissionGate
Controlled Shell Tool
```

下一步需要实现真实文件修改能力：

```text
write_file
edit_file
```

这是 Coding Agent 完成开发任务的核心能力。没有写入和编辑能力，Agent 只能读文件、搜索代码和运行测试，无法真正修复问题或完成用户要求的代码修改。

## 2. 设计目标

第一版目标：

- `write_file` 能写入 workspace 内文件。
- `edit_file` 能基于 old_text / new_text 修改 workspace 内文件。
- 写入和编辑必须是 `PermissionMode::Confirm`。
- 所有路径必须经过 WorkspaceGuard。
- 用户拒绝时不得落盘。
- 写入前由 PermissionGate 拦截，后续 TUI 可展示 path 和内容摘要。
- 工具返回可读的结构化文本结果。

## 3. 非目标

第一版不做：

- AST 级编辑。
- diff patch 解析。
- 多文件事务。
- 自动格式化。
- 自动 Git commit。
- 二进制文件编辑。
- 大文件分块写入。
- 复杂冲突解决。

这些后续在更高级的 CodeEdit Skill 或 PatchTool 中实现。

## 4. write_file

工具名：

```text
write_file
```

权限：

```cpp
PermissionMode::Confirm
```

参数：

```text
path: string
content: string
create_parent_dirs: string, default = "false"
```

行为：

1. 校验 `path` 不为空。
2. 使用 `Workspace.resolve(path)` 解析路径。
3. 如果路径逃逸 workspace，返回失败。
4. 如果 parent 不存在：
   - `create_parent_dirs == true` 时创建父目录。
   - 否则返回失败。
5. 如果目标是目录，返回失败。
6. 写入 `content`。
7. 返回写入路径和字节数。

返回示例：

```text
wrote file: src/main.cpp
bytes: 128
```

## 5. edit_file

工具名：

```text
edit_file
```

权限：

```cpp
PermissionMode::Confirm
```

参数：

```text
path: string
old_text: string
new_text: string
replace_all: string, default = "false"
```

行为：

1. 校验 `path` 不为空。
2. 校验 `old_text` 不为空。
3. 使用 `Workspace.resolve(path)` 解析路径。
4. 读取原文件。
5. 查找 `old_text`。
6. 如果未找到，返回失败。
7. 如果 `replace_all == true`，替换全部匹配。
8. 否则只替换第一次匹配。
9. 写回文件。
10. 返回替换次数和路径。

返回示例：

```text
edited file: README.md
replacements: 1
```

## 6. PermissionGate 集成

`write_file` 和 `edit_file` 都必须返回：

```cpp
PermissionMode::Confirm
```

因此执行前会经过 AgentRunner：

```text
Approve -> 执行写入/编辑
Deny -> 不执行，回传 User denied permission.
Edit -> 使用用户编辑后的参数执行
Feedback -> 不执行，把用户反馈回传模型
```

测试必须覆盖 Deny 不落盘。

## 7. 测试计划

新增：

```text
tests/test_write_edit_tools.cpp
```

覆盖：

- `test_write_file_approved_creates_file`
- `test_write_file_denied_not_created`
- `test_write_file_rejects_path_escape`
- `test_edit_file_approved_replaces_text`
- `test_edit_file_denied_not_modified`
- `test_edit_file_missing_old_text_fails`

## 8. 下一步

完成 Write / Edit Tools 后，Agent 就具备：

```text
读文件
搜索代码
写文件
编辑文件
执行受控 shell
权限确认
工具结果回传模型
```

下一步建议进入：

```text
feat: add session history and audit log
```

原因：交付要求明确要求记录用户输入、模型回复、工具调用、工具结果、权限拒绝和错误信息。此时工具体系已经基本成型，可以开始沉淀运行事件。
