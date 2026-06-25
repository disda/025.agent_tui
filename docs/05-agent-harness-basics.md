# Agent Harness 基础知识与实现思路

## 1. Agent Harness 是什么

本项目要开发的不是一个普通聊天窗口，而是一个 **Agent Harness**。

可以把 Harness 理解为：

```text
把模型、工具、权限、上下文、状态、日志、测试、安全边界串起来的运行时外壳。
```

模型只负责决策，Harness 负责让这个决策安全、可执行、可追踪、可测试。

最小 Agent 循环：

```text
用户输入
  ↓
构造 messages
  ↓
调用模型
  ↓
模型决定：直接回答 / 调用工具
  ↓
执行工具
  ↓
把工具结果作为 tool_result 回传模型
  ↓
模型继续推理
  ↓
直到完成
```

也就是：

```text
observe -> think -> act -> observe
```

## 2. 开发 Agent 需要掌握的基础知识

### 2.1 LLM 消息协议

模型看到的是一组 messages，而不是完整程序状态。

核心角色：

| Role | 说明 |
| --- | --- |
| system | 规则、Skill、项目上下文 |
| user | 用户输入 |
| assistant | 模型回复或 tool_call |
| tool | 工具执行结果 |

建议 C++ 数据结构：

```cpp
enum class Role {
    System,
    User,
    Assistant,
    Tool
};

struct Message {
    Role role;
    std::string content;
    std::string tool_call_id;
};
```

Agent 的“记忆”本质上就是如何组织和裁剪 messages。

### 2.2 Tool Calling

模型不能直接读文件、跑命令。模型只能提出结构化工具调用：

```json
{
  "name": "read_file",
  "arguments": {
    "path": "README.md"
  }
}
```

Harness 负责：

```text
解析 tool_call
找到工具
检查权限
执行工具
把结果塞回 messages
```

建议 C++ 抽象：

```cpp
enum class PermissionMode {
    Auto,
    Confirm
};

struct ToolResult {
    bool ok;
    std::string output;
    std::string error;
};

class Tool {
public:
    virtual ~Tool() = default;
    virtual std::string name() const = 0;
    virtual nlohmann::json schema() const = 0;
    virtual PermissionMode permission() const = 0;
    virtual ToolResult run(const nlohmann::json& args) = 0;
};
```

### 2.3 AgentRunner

`AgentRunner` 是 Harness 的核心。它不关心 TUI，也不关心用户配置界面，只负责模型和工具循环。

最小逻辑：

```cpp
AgentResult AgentRunner::run(std::vector<Message> messages) {
    for (int step = 0; step < max_loops_; ++step) {
        auto response = provider_.chat(messages, tool_registry_.schemas());

        if (response.is_text()) {
            return AgentResult::done(response.text);
        }

        for (auto call : response.tool_calls) {
            auto* tool = tool_registry_.find(call.name);

            if (!tool) {
                messages.push_back(tool_result(call.id, "Tool not found"));
                continue;
            }

            if (tool->permission() == PermissionMode::Confirm) {
                auto decision = approval_.request(call);

                if (decision.type == ApprovalType::Deny) {
                    messages.push_back(tool_result(call.id, "User denied permission."));
                    continue;
                }

                if (decision.type == ApprovalType::Edit) {
                    call.arguments = decision.edited_arguments;
                }
            }

            auto result = tool->run(call.arguments);
            messages.push_back(tool_result(call.id, result.ok ? result.output : result.error));
        }
    }

    return AgentResult::failed("Max loop count exceeded.");
}
```

这就是最小可用 Agent 内核。

### 2.4 ToolRegistry

`ToolRegistry` 负责工具管理：

- 注册工具
- 暴露 schema
- 根据名字找到工具
- 统一执行
- 统一处理错误

它需要处理：

- 模型调用不存在的工具
- 工具参数不合法
- 工具执行失败
- 工具结果太长
- 当前 Skill 是否允许使用该工具

建议接口：

```cpp
class ToolRegistry {
public:
    void register_tool(std::unique_ptr<Tool> tool);
    Tool* find(const std::string& name);
    std::vector<nlohmann::json> schemas() const;
    std::vector<nlohmann::json> schemas_for_skills(const std::vector<Skill>& skills) const;
};
```

### 2.5 PermissionGate

真实 Agent 和玩具 Agent 的重要分界是权限控制。

权限规则：

| 工具 | 权限 |
| --- | --- |
| list_dir | Auto |
| read_file | Auto |
| glob_files | Auto |
| search_text | Auto |
| write_file | Confirm |
| edit_file | Confirm |
| run_shell | Confirm |
| run_kwoa_cli 写操作 | Strong Confirm |

PermissionGate 不应只支持 yes/no，应预留：

```cpp
enum class ApprovalType {
    Approve,
    Deny,
    Edit,
    Feedback
};
```

原因：真实用户常见反馈不是简单同意或拒绝，而是：

```text
不要执行。
改一下命令再执行。
先解释一下为什么要执行。
只允许这一次。
```

权限拒绝必须作为 tool_result 回传模型，而不是静默丢弃：

```text
tool_result = "User denied permission."
```

### 2.6 SessionHistory

Session 不只是聊天记录，而是 Agent 运行轨迹。

必须记录：

- 用户输入
- 模型回复
- tool_call
- tool_result
- 权限请求
- 权限拒绝
- 错误信息
- 状态变化

运行时可使用：

```text
.agent-tui/sessions/session-xxx.jsonl
```

项目交付审计记录使用：

```text
.ai_history/logs/
```

两者职责不同：

| 目录 | 作用 |
| --- | --- |
| `.agent-tui/sessions/` | 运行时会话上下文 |
| `.ai_history/logs/` | L2 交付要求的 AI 协作记录和设计主导证据 |

### 2.7 ContextBuilder

ContextBuilder 负责构造模型输入。

它需要拼接：

- 系统规则
- 已加载 Skills
- 项目摘要
- 当前会话消息
- 工具结果
- 必要文件片段

第一版不要做复杂上下文压缩，只要遵守：

- 不要没读文件就猜内容
- 不要一次塞全仓库
- 按工具结果逐步构造上下文
- 工具结果过长时需要截断并说明

### 2.8 Skill Runtime

Skill 不是工具。

```text
Tool = 可执行函数
Skill = 什么时候用、怎么用、用哪些工具、注意什么
```

例如 `kwoa_cli` Skill 应该描述：

- 先检查 auth status
- 只读命令控制 count/page-limit
- 不打印 cookie
- 发送/撤回/转发必须确认
- 输出时说明数据来源和限制

Skill Runtime 负责：

- 发现 skill
- 读取 skill.yaml
- 读取 SKILL.md
- 根据用户输入选择 skill
- 限制可用工具集合
- 将 skill instruction 注入 system context

## 3. Harness 的推荐模块划分

```text
App
  负责启动、加载配置、组装依赖

TuiApp
  负责终端界面、输入、状态展示、权限面板

AgentLoop
  负责用户一轮输入、选择 Skill、管理 Session

AgentRunner
  负责 provider/tool loop

Provider
  负责模型协议

ToolRegistry
  负责工具注册和 schema

PermissionGate
  负责危险操作确认

SessionHistory
  负责上下文消息

AuditLog
  负责可追溯日志

SkillRuntime
  负责加载、选择和注入 Skills
```

运行链路：

```text
TuiApp 收到用户输入
  ↓
AgentLoop.add_user_message
  ↓
SkillSelector 选择 skills
  ↓
ContextBuilder 构造 messages
  ↓
AgentRunner.run
  ↓
Provider.chat
  ↓
模型返回 tool_calls
  ↓
PermissionGate 检查
  ↓
ToolRegistry 执行
  ↓
SessionHistory 记录 tool_result
  ↓
AgentRunner 继续请求模型
  ↓
最终答案返回 TuiApp
```

## 4. 第一阶段不需要掌握的内容

第一阶段不要做：

- 向量数据库
- 多 Agent
- MCP
- 浏览器自动化
- 长期记忆
- 复杂上下文压缩
- 云沙箱
- 插件市场
- 自动 Git 提交

这些都是后续扩展。当前目标是先把本地 Agent Kernel 写出来。

## 5. 推荐学习与实现顺序

### 第一步：C++ 工程骨架

```text
CMakeLists.txt
src/main.cpp
include/agent_tui/...
tests/...
```

### 第二步：核心数据结构

```text
Message
ToolCall
ToolResult
ProviderResponse
AgentResult
SessionEvent
```

### 第三步：MockProvider

不要先接真实模型。

MockProvider 应支持：

```text
第 1 次调用：返回 read_file tool_call
收到 tool_result 后
第 2 次调用：返回 Done
```

这样可以稳定测试 AgentRunner。

### 第四步：ToolRegistry

先做 fake 工具，再做真实工具：

```text
echo_tool
read_file
list_dir
search_text
run_shell
```

### 第五步：AgentRunner

跑通最小循环：

```text
MockProvider -> tool_call -> ToolRegistry -> tool_result -> MockProvider -> final answer
```

### 第六步：PermissionGate

先拦截：

```text
write_file
edit_file
run_shell
```

拒绝时必须回传：

```text
User denied permission.
```

### 第七步：SkillRuntime

加载：

```text
skills/*/skill.yaml
skills/*/SKILL.md
```

选择 skill 后，只暴露它允许的工具。

### 第八步：kwoa-cli Skill 验证

真实验收场景：

```text
用户：用 kwoa-cli 看看最近会话，不要发送消息
```

期望行为：

```text
命中 kwoa_cli skill
检查 auth status
未初始化就提示更新 cookie
已初始化才执行只读命令
发送消息必须强确认
```

### 第九步：TUI

最后补界面：

```text
Chat History
Tool Call Log
Status Bar
Permission Panel
Input Box
```

## 6. 最小知识地图

开发本项目需要补齐这些基础：

```text
C++20 基础
nlohmann/json
CMake
接口抽象 / 依赖注入
状态机
JSON Schema / function calling
进程执行 / stdout stderr exit code
文件路径安全 / std::filesystem
单元测试 / Mock
TUI 事件循环
```

不需要一开始完整学习 LangChain、MCP、多 Agent。

## 7. 一句话总结

本项目要实现的是：

```text
一个本地 Agent Harness：
模型负责决策，
工具负责执行，
PermissionGate 负责安全，
Session 负责上下文，
Skill 负责能力复用，
MockProvider 负责测试，
TUI 负责交互。
```

下一步应该进入 `minimal AgentRunner skeleton`，这是整个项目真正的地基。
