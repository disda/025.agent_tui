# Agent TUI

这是「L2命题任务-从零实现一个 TUI 终端编码 Agent（发布版）」的项目工作区。

## 文档入口

- [中文实施计划](./docs/00-implementation-plan.md)
- [题目 Markdown 原文](./output/l2-agent-tui-task.md)

## 目标

从零实现一个最小可用的本地 TUI 编码 Agent，覆盖 Agent Loop、结构化工具调用、权限确认、WPS CodingPlan 协议接入、会话上下文、配置管理、测试和交付物。

## 交付关注点

- 程序必须以 TUI 形式运行。
- Agent Loop、工具系统、权限控制、会话管理必须自行实现。
- 只读工具可自动执行，写文件、编辑文件、Shell 命令必须经过用户确认。
- 关键 AI 协作记录需要沉淀到 `.ai_history/logs/`。
- 可运行验证产物需要放到 `deliverables/`。
