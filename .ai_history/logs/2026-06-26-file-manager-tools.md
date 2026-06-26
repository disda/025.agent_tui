# AI 协作记录：Safe File Manager Tools

日期：2026-06-26

## 背景

用户要求按照 TODO 优先级先开发功能类。当前最高优先级是 P0：安全本地文件管理能力，用于支持：

```text
列出桌面文件
移动图片到新文件夹
整理下载目录
```

## 本轮技术文档

新增：

```text
docs/25-file-manager-tools-design.md
docs/26-file-manager-tools-verification.md
```

## 本轮实现

新增：

```text
include/agent_tui/filesystem/KnownPaths.hpp
include/agent_tui/filesystem/AllowedRoots.hpp
include/agent_tui/tools/FileManagerTools.hpp
tests/test_file_manager_tools.cpp
```

更新：

```text
CMakeLists.txt
TODO.md
```

## 当前能力

### KnownPaths

支持：

```text
home
desktop
downloads
documents
pictures
```

### AllowedRoots

支持：

```text
workspace_only
with_known_user_dirs
resolve
is_allowed
```

所有路径必须位于 allowed roots 内。

### FileManagerTools

新增工具：

```text
list_path
make_dir
move_file
move_files_by_extension
```

权限策略：

```text
list_path -> Auto
make_dir -> Confirm
move_file -> Confirm
move_files_by_extension -> Confirm
```

批量移动支持 dry-run：

```text
execute=false
```

默认只生成计划，不移动文件。

## 测试覆盖

新增测试：

```text
tests/test_file_manager_tools.cpp
```

覆盖：

- KnownPaths alias。
- AllowedRoots 越权拒绝。
- list_path。
- make_dir。
- move_file。
- move_file target conflict。
- move_files_by_extension dry-run。
- move_files_by_extension execute。
- move_files_by_extension target escape rejection。

## 下一步建议

下一步进入：

```text
feat: route desktop file intents to file manager tools
```

目标：让 TUI 能直接处理：

```text
列出桌面文件
移动图片到新文件夹
```

并先展示 dry-run 计划，再确认执行。
