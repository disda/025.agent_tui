# -*- coding: utf-8 -*-
"""
Minimal Markdown reader for WPS/KDocs documents.

This is intentionally based on the MCP helpers from the provided md_to_otl.py
snippet: same endpoint, token resolution, JSON-RPC initialize, and tools/call
flow. It only implements the `get` path needed for this task.
"""

from __future__ import annotations

import argparse
import json
import os
import sys
import time
import urllib.error
import urllib.request
from pathlib import Path

MCP_ENDPOINT = "https://mcp-center.wps.cn/skill_hub/mcp"
SKILL_VERSION = "1.0.1"
DEFAULT_PROTOCOL_VERSION = "2025-03-26"

_REQ_ID = 0
_SESSION_ID: str | None = None
_DOTENV_CACHE: dict[str, str] | None = None


def _next_id() -> int:
    global _REQ_ID
    _REQ_ID += 1
    return _REQ_ID


def _load_dotenv() -> dict[str, str]:
    global _DOTENV_CACHE
    if _DOTENV_CACHE is not None:
        return _DOTENV_CACHE

    values: dict[str, str] = {}
    env_path = Path(__file__).resolve().parent.parent / ".env"
    if not env_path.is_file():
        _DOTENV_CACHE = values
        return values

    for raw_line in env_path.read_text(encoding="utf-8-sig").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#") or "=" not in line:
            continue
        key, value = line.split("=", 1)
        key = key.strip()
        value = value.strip()
        if not key:
            continue
        if len(value) >= 2 and value[0] == value[-1] and value[0] in {"'", '"'}:
            value = value[1:-1]
        values[key] = value

    _DOTENV_CACHE = values
    return values


def _resolve_token(token: str) -> str:
    dotenv = _load_dotenv()
    resolved = (
        token
        or os.environ.get("KDOC_TOKEN", "")
        or os.environ.get("KINGSOFT_DOCS_TOKEN", "")
        or dotenv.get("KDOC_TOKEN", "")
        or dotenv.get("KINGSOFT_DOCS_TOKEN", "")
    )
    if not resolved:
        raise RuntimeError(
            "Missing token. Pass --token or set KDOC_TOKEN / KINGSOFT_DOCS_TOKEN, "
            "or store one of them in .env."
        )
    return resolved


def _decode_response(raw: bytes) -> dict:
    text = raw.decode("utf-8")
    try:
        return json.loads(text)
    except json.JSONDecodeError:
        # Some MCP servers answer as text/event-stream. Accept the final JSON
        # payload from `data:` lines without changing the public call contract.
        payloads = []
        for line in text.splitlines():
            line = line.strip()
            if line.startswith("data:"):
                payload = line[5:].strip()
                if payload and payload != "[DONE]":
                    payloads.append(payload)
        for payload in reversed(payloads):
            try:
                return json.loads(payload)
            except json.JSONDecodeError:
                continue
        raise


def _mcp_post(payload: dict, token: str) -> tuple[dict, str | None]:
    body = json.dumps(payload, ensure_ascii=False).encode("utf-8")
    headers = {
        "Content-Type": "application/json; charset=utf-8",
        "Accept": "application/json, text/event-stream",
        "Authorization": f"Bearer {token}",
        "X-Skill-Version": SKILL_VERSION,
    }
    if _SESSION_ID:
        headers["Mcp-Session-Id"] = _SESSION_ID

    request = urllib.request.Request(MCP_ENDPOINT, data=body, headers=headers)
    try:
        with urllib.request.urlopen(request, timeout=60) as response:
            session_id = response.headers.get("Mcp-Session-Id")
            return _decode_response(response.read()), session_id
    except urllib.error.HTTPError as exc:
        message = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"HTTP {exc.code}: {message}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(f"Request failed: {exc}") from exc


def _mcp_initialize(token: str) -> None:
    global _SESSION_ID
    payload = {
        "jsonrpc": "2.0",
        "id": _next_id(),
        "method": "initialize",
        "params": {
            "protocolVersion": DEFAULT_PROTOCOL_VERSION,
            "capabilities": {},
            "clientInfo": {"name": "read-kdocs-markdown", "version": "1.0.0"},
        },
    }
    data, session_id = _mcp_post(payload, token)
    if session_id:
        _SESSION_ID = session_id
    if "error" in data:
        raise RuntimeError(f"MCP initialize failed: {data['error']}")

    try:
        _mcp_post({"jsonrpc": "2.0", "method": "notifications/initialized"}, token)
    except Exception:
        pass


def _mcp_call(method: str, params: dict, token: str = "") -> dict:
    global _SESSION_ID
    resolved_token = _resolve_token(token)
    if not _SESSION_ID:
        _mcp_initialize(resolved_token)

    payload = {
        "jsonrpc": "2.0",
        "id": _next_id(),
        "method": "tools/call",
        "params": {"name": method, "arguments": params},
    }
    data, session_id = _mcp_post(payload, resolved_token)
    if session_id:
        _SESSION_ID = session_id
    if "error" in data:
        raise RuntimeError(f"MCP error: {data['error']}")

    result = data.get("result", {})
    for item in result.get("content", []):
        if item.get("type") != "text":
            continue
        text = item.get("text", "")
        try:
            return json.loads(text)
        except (TypeError, json.JSONDecodeError):
            return {"raw": text}
    return result


def _unwrap_data(response: dict) -> dict:
    data = response.get("data", response)
    if isinstance(data, dict) and "data" in data and isinstance(data["data"], dict):
        return data["data"]
    return data


def _extract_markdown(response: dict) -> tuple[str, list[str]]:
    data = _unwrap_data(response)
    warnings = data.get("warnings") or response.get("warnings") or []
    if not isinstance(warnings, list):
        warnings = [str(warnings)]

    for key in ("markdown", "content", "text", "raw"):
        value = data.get(key) if isinstance(data, dict) else None
        if isinstance(value, str) and value.strip():
            return value, warnings

    if isinstance(data, dict):
        for key in ("result", "document"):
            nested = data.get(key)
            if isinstance(nested, dict):
                for nested_key in ("markdown", "content", "text"):
                    value = nested.get(nested_key)
                    if isinstance(value, str) and value.strip():
                        return value, warnings

    raise RuntimeError(
        "read_file returned no recognizable Markdown field. "
        f"Top-level keys: {sorted(response.keys())}"
    )


def read_markdown(url: str, token: str = "", poll_interval: float = 2.0, max_polls: int = 60) -> tuple[str, list[str]]:
    params: dict[str, str] = {"url": url}
    last_response: dict | None = None

    for poll_index in range(max_polls + 1):
        response = _mcp_call("read_file", params, token)
        last_response = response
        data = _unwrap_data(response)
        status = str(data.get("status", response.get("status", ""))).lower() if isinstance(data, dict) else ""
        task_id = data.get("task_id") if isinstance(data, dict) else None

        if status in {"pending", "running", "processing"} and task_id:
            params["task_id"] = str(task_id)
            time.sleep(poll_interval)
            continue

        return _extract_markdown(response)

    raise RuntimeError(f"read_file did not complete after {max_polls} polls: {last_response}")


def main() -> int:
    parser = argparse.ArgumentParser(description="Read a KDocs document as Markdown.")
    parser.add_argument("--url", required=True, help="KDocs/WPS document URL")
    parser.add_argument("-o", "--output", required=True, help="Output Markdown path")
    parser.add_argument("--token", default="", help="Optional token override")
    args = parser.parse_args()

    markdown, warnings = read_markdown(args.url, args.token)
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    output_path.write_text(markdown.rstrip() + "\n", encoding="utf-8")

    print(json.dumps({"output": str(output_path), "warnings": warnings}, ensure_ascii=False))
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(1)
