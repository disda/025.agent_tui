# 构建验证记录：FileTools + WorkspaceGuard

日期：2026-06-26

## 1. 验证目标

本次验证目标是实现真实只读文件工具，并为后续 `run_shell`、`write_file`、`edit_file`、SkillRuntime 提供 workspace 安全边界。

新增内容：

```text
include/agent_tui/workspace/Workspace.hpp
include/agent_tui/tools/FileTools.hpp
tests/test_file_tools.cpp
```

更新：

```text
CMakeLists.txt
```

## 2. 新增能力

### WorkspaceGuard

`Workspace` 负责：

- 保存 workspace root。
- 将用户输入路径解析为 canonical path。
- 阻止 `../` path traversal。
- 确保工具只能访问 workspace 内路径。
- 将绝对路径转成相对展示路径。

### FileTools

新增只读工具：

```text
list_dir
read_file
glob_files
search_text
```

权限：

```text
PermissionMode::Auto
```

这些工具不需要用户确认，但必须受 WorkspaceGuard 限制。

## 3. 测试覆盖

`tests/test_file_tools.cpp` 覆盖：

- Workspace 拒绝 `../outside.txt`。
- `list_dir` 能列出 workspace 内目录。
- `read_file` 能读取 workspace 内文件。
- `read_file` 拒绝 path traversal。
- `glob_files` 能匹配文件。
- `search_text` 能搜索文本，并跳过 build 目录。

## 4. 本地隔离构建环境

C++ 项目使用独立 `build/` 目录作为隔离构建环境。

验证命令：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 5. 验证结果

本地沙箱验证通过：

```text
-- Configuring done
-- Generating done
-- Build files have been written to: /mnt/data/agent_tui_impl/build
[100%] Built target agent_tui_file_tools_tests
Test project /mnt/data/agent_tui_impl/build
    Start 1: agent_tui_tests
1/2 Test #1: agent_tui_tests ..................   Passed    0.02 sec
    Start 2: agent_tui_file_tools_tests
2/2 Test #2: agent_tui_file_tools_tests .......   Passed    0.02 sec

100% tests passed, 0 tests failed out of 2
```

## 6. 当前限制

当前 FileTools 仍是第一版：

- `glob_files` 使用简单 glob 转 regex。
- `search_text` 暂未做编码检测。
- `search_text` 只做纯文本子串搜索。
- 输出截断策略较简单。
- 暂未实现 binary 文件跳过判断。

## 7. 下一步

下一步建议实现：

```text
feat: add permission gate
```

之后再实现：

```text
feat: add controlled shell tool
```

原因：`run_shell` 是高风险工具，必须先有 PermissionGate。
