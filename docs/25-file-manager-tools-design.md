# Safe File Manager Tools 技术设计

## 1. 背景

用户开始提出非仓库内任务：

```text
列出桌面文件
移动图片到新文件夹
整理下载目录
```

这类任务不适合直接复用 `WorkspaceGuard`。`WorkspaceGuard` 的职责是保护代码仓库，不应被扩展成全盘文件权限。

因此需要新增一套安全本地文件管理能力：

```text
KnownPaths
AllowedRoots
FileManagerTools
```

## 2. 设计目标

第一版目标：

- 支持识别用户常见目录：home、desktop、downloads、documents、pictures。
- 支持独立的 allowed roots 安全边界。
- 支持列出目录。
- 支持创建目录。
- 支持移动单个文件。
- 支持按扩展名批量移动文件。
- 支持 dry-run 预览，默认不执行批量移动。
- 所有写操作 / 移动操作由工具层声明为 `PermissionMode::Confirm`。

## 3. 非目标

第一版不做：

- 永久删除文件。
- 递归全盘扫描。
- 跨磁盘复杂事务。
- 文件冲突自动覆盖。
- 回收站集成。
- 图片内容识别。

## 4. KnownPaths

新增：

```text
include/agent_tui/filesystem/KnownPaths.hpp
```

能力：

```cpp
KnownPaths::home()
KnownPaths::desktop()
KnownPaths::downloads()
KnownPaths::documents()
KnownPaths::pictures()
KnownPaths::resolve_alias("desktop")
```

Windows 使用 `USERPROFILE`，POSIX 使用 `HOME`。

## 5. AllowedRoots

新增：

```text
include/agent_tui/filesystem/AllowedRoots.hpp
```

能力：

- 保存允许访问的根目录集合。
- `resolve(alias_or_path)` 解析路径。
- `is_allowed(path)` 判断 canonical path 是否位于 allowed roots 内。
- 默认允许 workspace。
- 可选允许 known paths。

## 6. FileManagerTools

新增：

```text
include/agent_tui/tools/FileManagerTools.hpp
```

工具：

```text
list_path
make_dir
move_file
move_files_by_extension
```

### list_path

只读，自动执行。

参数：

```text
path
max_entries
```

### make_dir

写操作，需要确认。

参数：

```text
path
```

### move_file

移动单文件，需要确认。

参数：

```text
source
target
```

不覆盖已有文件。

### move_files_by_extension

批量移动，需要确认。

参数：

```text
source_dir
target_dir
extensions
execute
```

`execute=false` 时只生成 dry-run 计划。

## 7. 安全策略

- 所有路径必须位于 allowed roots 内。
- 默认不允许访问系统目录。
- 批量移动默认 dry-run。
- 不覆盖目标文件。
- 不递归扫描子目录。
- 移动 / 创建目录需要 PermissionGate 或 TUI 确认。

## 8. 测试计划

新增：

```text
tests/test_file_manager_tools.cpp
```

覆盖：

- known path alias 解析。
- allowed root 允许 workspace 内路径。
- allowed root 拒绝越权路径。
- list_path 列出文件。
- make_dir 创建目录。
- move_file 移动文件。
- move_files_by_extension dry-run 不移动。
- move_files_by_extension execute 移动指定扩展名。
- 目标冲突时失败。
