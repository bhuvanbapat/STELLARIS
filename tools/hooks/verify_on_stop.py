"""Claude Code Stop hook: incremental build + unit tests before a turn ends.

Registered in .claude/settings.json. Cheap by design:
  * does nothing until the CMake build directory has been configured;
  * does nothing if no source file changed since the last successful check
    (timestamp in build/.claude-verify-stamp);
  * otherwise runs an incremental `cmake --build` and `ctest`.

On failure it exits with code 2 and prints only the decisive error lines to
stderr, which Claude Code feeds back to Claude so the turn cannot end with a
broken build. If Claude is already continuing because of this hook
(stop_hook_active), it lets the stop through to avoid an infinite loop.
"""

import json
import os
import subprocess
import sys
from pathlib import Path

PROJECT = Path(os.environ.get("CLAUDE_PROJECT_DIR", Path(__file__).resolve().parents[2]))
BUILD = PROJECT / "build"
STAMP = BUILD / ".claude-verify-stamp"
CONFIG = "Release"
WATCHED = ["app", "tests", "glut_demo", "cmake", "third_party/glad", "CMakeLists.txt"]
SOURCE_SUFFIXES = {".cpp", ".h", ".hpp", ".c", ".txt", ".cmake", ".glsl", ".vert", ".frag", ".comp"}
MAX_ERROR_LINES = 40


def newest_source_mtime() -> float:
    newest = 0.0
    for entry in WATCHED:
        path = PROJECT / entry
        if path.is_file():
            newest = max(newest, path.stat().st_mtime)
        elif path.is_dir():
            for f in path.rglob("*"):
                if f.suffix.lower() in SOURCE_SUFFIXES and f.is_file():
                    newest = max(newest, f.stat().st_mtime)
    return newest


def run(cmd: list[str], timeout: int) -> tuple[int, str]:
    try:
        proc = subprocess.run(cmd, cwd=PROJECT, capture_output=True, text=True,
                              errors="replace", timeout=timeout)
        return proc.returncode, proc.stdout + proc.stderr
    except subprocess.TimeoutExpired:
        return 1, f"timed out after {timeout}s: {' '.join(cmd)}"
    except OSError as exc:
        return 1, f"could not run {cmd[0]}: {exc}"


def decisive_lines(output: str) -> str:
    keys = ("error", "failed", "fatal", "***")
    lines = [l for l in output.splitlines() if any(k in l.lower() for k in keys)]
    if not lines:
        lines = output.splitlines()[-MAX_ERROR_LINES:]
    return "\n".join(lines[:MAX_ERROR_LINES])


def main() -> int:
    try:
        payload = json.load(sys.stdin)
    except (json.JSONDecodeError, ValueError):
        payload = {}
    if payload.get("stop_hook_active"):
        return 0
    if not (BUILD / "CMakeCache.txt").exists():
        return 0

    newest = newest_source_mtime()
    if STAMP.exists() and STAMP.stat().st_mtime >= newest:
        return 0

    code, out = run(["cmake", "--build", str(BUILD), "--config", CONFIG, "--parallel"], timeout=540)
    if code != 0:
        print("STELLARIS verify hook: incremental build FAILED\n" + decisive_lines(out), file=sys.stderr)
        return 2

    code, out = run(["ctest", "--test-dir", str(BUILD), "-C", CONFIG, "--output-on-failure"], timeout=120)
    if code != 0:
        print("STELLARIS verify hook: unit tests FAILED\n" + decisive_lines(out), file=sys.stderr)
        return 2

    STAMP.touch()
    return 0


if __name__ == "__main__":
    sys.exit(main())
