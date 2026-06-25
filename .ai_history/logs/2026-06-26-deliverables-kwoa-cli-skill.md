# AI 协作记录：Deliverables 与 kwoa-cli Skill 验证目标

日期：2026-06-26

## 背景

用户补充 L2 交付要求：

1. 项目源码仓库：可编译运行的 TUI Agent 完整源码。
2. AI 协作过程记录：`.ai_history/logs/` 必须提交至源码仓库。
3. 可运行验证产物：放置在源码目录 `deliverables/` 下。
4. Coding Agent 应能在真实环境运行，并完成读取项目文件、搜索代码、修改文件、执行测试命令，并根据工具结果继续完成任务。
5. 测试至少覆盖 Agent 主循环、工具调用与结果回传、权限确认与拒绝、配置优先级和 Mock LLM Provider。
6. 可运行验证产物应包含关键运行截图。

用户同时明确项目最终验证方向：

```text
加载 kwoa-cli Skill，
实现 IM 文档以及 KDocs 文档操作等功能。
```

## 本轮动作

新增：

```text
deliverables/README.md
deliverables/demo-kwoa-cli-skill.md
deliverables/screenshots/.gitkeep
```

更新：

```text
README.md
TODO.md
```

## Deliverables 定位

`deliverables/` 用于放置 L2 要求中的可运行验证产物。

当前验证目标：

```text
加载 kwoa-cli Skill，
理解 kwoa-cli 的 IM / KDocs 文档操作规则，
调用只读工具读取 IM / 文档信息，
对消息发送、撤回、转发、文档写入等高风险操作进行权限确认，
把工具结果回传模型继续推理，
最后输出有数据来源和限制说明的结果。
```

## Demo 场景

### 场景 A：kwoa-cli Skill 只读 IM 查询

用户任务：

```text
用 kwoa-cli 看看最近会话，不要发送消息。
```

期望：命中 kwoa_cli Skill，检查 auth status，认证通过后执行 recent-list，只读输出总结。

### 场景 B：KDocs 文档读取

用户任务：

```text
读取这个 KDocs 文档并总结内容，不要写入。
```

期望：调用 docs +info / docs +read，读取结果后总结内容，不执行写入。

### 场景 C：写操作权限拦截

用户任务：

```text
给 chat 123 发送 hello。
```

期望：识别为高风险写操作，展示命令，请求确认；未确认不执行，并将拒绝作为 tool_result 回传模型。

## 下一步建议

继续推进：

```text
feat: add file tools and workspace guard
```

然后再实现：

```text
PermissionGate
run_shell
SkillRuntime
kwoa_cli Skill
run_kwoa_cli / safe shell wrapper
```
