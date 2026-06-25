# Skill 标准（kwoa-cli 风格）

本项目采用 kwoa-cli 风格的 Skills Runtime 组织 Agent 能力。

## 1. Skill 定义

Skill 是描述能力的运行时单元，不直接执行代码，只定义：

- 能做什么
- 什么时候触发
- 允许使用哪些工具
- 给模型的行为约束

## 2. 目录结构

```text
skills/
  repo_reader/
    skill.yaml
    SKILL.md
  code_editor/
    skill.yaml
    SKILL.md
  shell_runner/
    skill.yaml
    SKILL.md
  cpp_project/
    skill.yaml
    SKILL.md
  tui_agent/
    skill.yaml
    SKILL.md
```

## 3. skill.yaml 示例

```yaml
id: repo_reader
name: Repository Reader
description: Read project files, list directories and search text.
triggers:
  - read project
  - inspect code
  - search file
  - understand repository
allowed_tools:
  - list_dir
  - read_file
  - glob_files
  - search_text
permission: read_only
priority: 10
```

## 4. SKILL.md 示例

```md
# Repository Reader Skill

Use this skill when the user asks to inspect, understand, summarize, or search the current repository.

Rules:
- Prefer list_dir before reading unknown paths.
- Use search_text for symbols, class names, function names and TODOs.
- Do not guess file content.
- Always base conclusions on tool results.
```

## 5. Skill 与 Tool 的区别

| 类型 | 作用 |
| --- | --- |
| Skill | 能力描述、触发条件、工具集合、模型行为规范 |
| Tool | 本地可执行结构化函数 |

模型不会直接执行 Skill。模型在 Skill 指令约束下调用 Tool。

## 6. Skill Selector

第一版不使用向量数据库，直接做关键词匹配：

1. 遍历所有 Skill。
2. 用用户输入匹配 `triggers`。
3. 命中后按 `priority` 排序。
4. 最多加载 4 个 Skill。
5. 没命中时 fallback 到默认 Skills。

默认 Skills：

- `repo_reader`
- `code_editor`
- `shell_runner`

## 7. Prompt 组装

只将选中的 Skill 注入系统上下文：

```text
You are a local terminal coding agent.

Rules:
- Do not claim file contents without reading them.
- Use read-only tools freely.
- Ask for confirmation before write/edit/shell tools.
- If permission is denied, continue safely.
- Keep responses concise and grounded in tool results.

Loaded skills:
<repo_reader/SKILL.md>
<code_editor/SKILL.md>
<shell_runner/SKILL.md>
```

## 8. 第一批内置 Skills

### repo_reader

用途：读取仓库结构、文件内容和搜索文本。

允许工具：

- list_dir
- read_file
- glob_files
- search_text

### code_editor

用途：修改文件和局部替换。

允许工具：

- read_file
- write_file
- edit_file

### shell_runner

用途：运行构建、测试、格式化等 shell 命令。

允许工具：

- run_shell

### cpp_project

用途：理解和操作 C++ / CMake 项目。

允许工具：

- list_dir
- read_file
- search_text
- run_shell

### tui_agent

用途：本项目自身架构开发，包括 Agent Loop、TUI、Provider、Tool System。

允许工具：

- list_dir
- read_file
- search_text
- write_file
- edit_file
- run_shell
