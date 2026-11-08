# Claude Code setup for STELLARIS (audit of 2026-11-02)

What was verified, what was changed, and what is unavailable. Configuration
changes are project-scoped (inside this repository) unless stated otherwise.

## Context and usage (verified)

- **Auto-compaction:** active by default (no `autoCompactEnabled` override in
  `~/.claude.json`). This session reports a 1,000,000-token window that
  auto-compacts at 97 % (via the app's usage tool). No extra compaction tool
  was installed.
- **Inspection commands:** `/context` (context-window breakdown), `/compact`
  (manual compaction, with optional focus text), `/usage` (plan limits), and
  `/cost` (session cost; mainly meaningful for API-key billing). In the desktop
  app, the usage card shows the same plan limits.
- **Persistence:** decisions and milestone results go in
  `DEVELOPMENT_LOG.md`, which CLAUDE.md §18 tells every session to read.

## Plugins (verified with `claude plugin list`, CLI 2.1.309)

| Plugin | Installed | Enabled | Notes |
|---|---|---|---|
| context7 | yes | yes | Its MCP server reported `needs_auth` early in the session and `connected` (2 tools) later the same session. |
| feature-dev | yes | yes | Agents `code-explorer`, `code-architect`, `code-reviewer` are available. |
| pr-review-toolkit | yes | yes | Six review agents available. |
| code-simplifier | yes | yes | — |

None were changed. The user's model (Opus 5.5) and effort (High) are app
settings; no file changed by this audit touches them. `~/.claude/settings.json`
contains no `model` or effort key and was not modified.

## MCP servers / connectors available to this session (verified)

| Server | Status |
|---|---|
| Claude Docs | connected (8 tools) |
| scheduled-tasks | connected (6 tools) |
| context7 (plugin) | connected (2 tools), re-checked later in the session |

Also available: the built-in browser pane and the desktop terminal tools.
Claude in Chrome tools are listed but have not been exercised (unverified).

## Skills added (project-scoped: `.claude/skills/`)

See `.claude/skills/README.md` for sources, commits, licenses and local edits.

| Skill | Purpose | Invocation |
|---|---|---|
| systematic-debugging | root-cause-first debugging process | automatic |
| verification-before-completion | evidence before any "done/passing" claim | automatic |
| shader-dev | GLSL technique library (ray marching, volumetrics, noise, particles, post) | automatic or `/shader-dev` |
| caveman | terse output style to cut output tokens | **only** via `/caveman`; `stop caveman` ends it |

**Operational status:** the files are installed and their frontmatter has been
validated. They are **not active in the session that installed them**, because
the `.claude/skills` folder was created after that session started. Run
`/reload-skills` or start a new session to load them. A headless check failed
because the bundled CLI is not logged in, so it could not confirm loading.

Isolated subagents: available through the Agent tool (general-purpose, Explore,
Plan, feature-dev and pr-review-toolkit agents; worktree isolation supported).

## Automation added

`.claude/settings.json` registers one **Stop** hook,
`tools/hooks/verify_on_stop.py`:

- does nothing until `build/` has been configured;
- does nothing if no source file changed since its last successful run
  (stamp file `build/.claude-verify-stamp`), so a turn without code changes
  costs nothing;
- otherwise runs an incremental `cmake --build build --config Release` and
  `ctest`; on failure it returns only the decisive error lines and blocks the
  turn from ending;
- respects `stop_hook_active` so it can never loop.

Tested so far: the early-exit paths (no build directory; loop guard).

**Considered and rejected:**
- Format-on-edit hook (clang-format): it rewrites files after every edit, which
  invalidates the editor's view of the file and forces re-reads. That costs
  more tokens than it saves.
- Build-on-every-edit hook: too expensive, as the task required.
- Shader validation hook: no offline GLSL validator (glslang) is installed.
  The app validates all shaders at startup instead.

## Deliberately not installed

The full superpowers plugin (session-start hook plus redundant workflow skills),
the full caveman plugin (always-on hooks), frontend-design (web only),
clangd-lsp (no clangd binary), and any extra compaction tool. Reasons are in
`.claude/skills/README.md`.

## How to undo

Delete `.claude/skills/<name>/` to remove a skill. Delete `.claude/settings.json`
(or its `Stop` entry) and `tools/hooks/` to remove the hook. Remove §18 from
CLAUDE.md.
