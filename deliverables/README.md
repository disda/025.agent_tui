# Deliverables

本目录用于放置 L2 交付要求中的可运行验证产物。

## 1. 交付清单对应关系

| 要求 | 本项目对应位置 |
| --- | --- |
| 项目源码仓库 | 仓库根目录源码、`include/`、`src/`、`tests/`、`CMakeLists.txt` |
| AI 协作过程记录 | `.ai_history/logs/` |
| 可运行验证产物 | `deliverables/` |

## 2. 本项目验证目标

本项目的最终验证任务不是做普通玩具 Demo，而是验证一个真实 Coding Agent Harness：

```text
加载 kwoa-cli Skill，
理解 kwoa-cli 的 IM / KDocs 文档操作规则，
调用只读工具读取 IM / 文档信息，
对消息发送、撤回、转发、文档写入等高风险操作进行权限确认，
把工具结果回传模型继续推理，
最后输出有数据来源和限制说明的结果。
```

## 3. 计划验证场景

### 场景 A：kwoa-cli Skill 只读 IM 查询

用户任务：

```text
用 kwoa-cli 看看最近会话，不要发送消息。
```

期望 Agent 行为：

1. SkillSelector 命中 `kwoa_cli`。
2. 加载 kwoa-cli Skill 规则。
3. 调用 `auth status` 检查本地认证状态。
4. 如果未初始化，提示用户更新 cookie，不猜测数据。
5. 如果已初始化，执行只读 recent-list。
6. 输出命令、数据来源、页数限制、结论。

### 场景 B：kwoa-cli KDocs 文档读取

用户任务：

```text
读取指定 KDocs 文档并总结内容，不要写入。
```

期望 Agent 行为：

1. 识别这是 kwoa-cli docs 只读流程。
2. 调用 `docs +info` 或 `docs +read`。
3. 将结果回传模型。
4. 输出总结，并说明 file_id/link_id、读取命令和限制。

### 场景 C：kwoa-cli 写操作权限拦截

用户任务：

```text
给 chat 123 发送 hello。
```

期望 Agent 行为：

1. 识别 `im +messages-send --yes` 属于高风险写操作。
2. 先展示命令或 dry-run。
3. 请求用户明确确认。
4. 未确认时不执行，并把拒绝作为 tool_result 回传模型。
5. 确认后才执行真实写操作。

## 4. 当前已完成验证

当前阶段已完成 C++ 项目地基验证：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

验证记录见：

```text
docs/06-build-verification.md
```

## 5. 后续需要补充的交付材料

随着功能实现推进，本目录后续应补充：

```text
deliverables/demo-kwoa-cli-skill.md
deliverables/run-log.md
deliverables/screenshots/*.png
```

其中截图应覆盖：

- TUI 启动
- 用户输入 kwoa-cli 任务
- Tool Call 展示
- Permission Panel 展示
- 工具结果展示
- 最终回答展示
