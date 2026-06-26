# AI 协作记录：作业要求缺口与 TODO 更新

日期：2026-06-26

## 背景

用户要求对照 L2 作业要求，检查当前项目还缺失哪些能力，并据此先更新 TODO。

## 本轮判断

当前已完成较多 Agent Harness 地基：

```text
AgentRunner
ToolRegistry
MockProvider
FileTools + WorkspaceGuard
PermissionGate
Controlled Shell Tool
Write / Edit Tools
```

但对照作业要求，仍缺关键交付项：

```text
TUI
SessionHistory / AuditLog
ConfigLoader / 配置优先级
SkillRuntime
kwoa_cli Skill 验证
AgentLoop 应用层
OpenAI-compatible Provider
WPS CodingPlan Provider
deliverables 真实运行日志和截图
```

## 本轮更新

已更新 `TODO.md`，新增“作业要求对齐状态”表，将作业要求逐项映射到当前状态和缺口。

同时将后续优先级调整为：

```text
1. SessionHistory / AuditLog
2. ConfigLoader / 配置优先级
3. SkillRuntime + kwoa-cli Skill 验证
4. AgentLoop 应用层
5. Minimal TUI
6. Provider / CodingPlan
7. Deliverables 真实验证产物
```

## 下一步建议

下一步仍然是：

```text
feat: add session history and audit log
```

原因：作业明确要求用户输入、模型回复、工具调用、工具执行结果、权限拒绝结果和错误信息都进入会话历史；后续 TUI、AgentLoop 和 Deliverables 都依赖这一层。
