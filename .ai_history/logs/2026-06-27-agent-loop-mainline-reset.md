# 2026-06-27 Agent Loop Mainline Reset

## Context

The project drifted toward local intent rules, desktop file management, and kwoa-cli style Skills Runtime. The L2 assignment goal is narrower and more important: implement a TUI coding Agent that can use model tool calls to inspect, edit, and run code.

## Decision

Reset P0 to the model-driven coding Agent loop:

```text
user prompt
  -> TUI
  -> Provider text/tool_calls
  -> AgentRunner
  -> Permission approval for write/edit/shell
  -> ToolRegistry execution
  -> tool_result back to Provider
  -> final answer
```

Local Intent Router remains only as a temporary shortcut/fallback and should not be expanded as the main natural-language understanding mechanism.

## Implemented

- Added `mock-agent-demo` provider for local no-API tool-loop verification.
- Routed ordinary TUI prompts through `AgentRunner`.
- Added TUI approval service for confirmed tools.
- Registered core coding tools in the TUI AgentRunner path.
- Added a TUI test proving a prompt can trigger `write_file` and generate `demo.py`.
- Updated README, TODO, and `docs/00-implementation-plan.md` to put the coding Agent loop first.

## Verification

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

Result: 11/11 tests passed.

Manual smoke test:

```bash
printf '/api provider mock-agent-demo\n实现一个简单代码 demo\ny\n/exit\n' | ./build/agent_tui
```

Result: TUI requested `write_file` approval and generated `demo.py` containing `print('hello from agent_tui')`.
