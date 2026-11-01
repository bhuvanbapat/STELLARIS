# Project skills (STELLARIS)

Project-scoped Claude Code skills. Claude Code loads `.claude/skills/<name>/SKILL.md`
automatically for sessions started in this repository (see the official docs at
https://code.claude.com/docs/en/skills). Delete a folder to uninstall that skill.

| Skill | Source (commit) | License | Invocation | Local changes |
|---|---|---|---|---|
| `systematic-debugging` | [obra/superpowers](https://github.com/obra/superpowers) `skills/systematic-debugging` @ `2ce8aa15d05b` | MIT | automatic when a bug/build/test failure appears | Two cross-references to other superpowers skills replaced with the STELLARIS CTest suite and the local `verification-before-completion` skill. Pressure-test files, creation log and TypeScript/shell examples were not copied. |
| `verification-before-completion` | obra/superpowers `skills/verification-before-completion` @ `2ce8aa15d05b` | MIT | automatic before completion claims | None. |
| `shader-dev` | [MiniMax-AI/skills](https://github.com/MiniMax-AI/skills) `skills/shader-dev` @ `60aaae52bb2a` | MIT | automatic for GLSL work, or `/shader-dev` | None. Written for ShaderToy/WebGL2 GLSL; STELLARIS uses desktop GLSL 4.50, so its WebGL2 adaptation rules do not apply. |
| `caveman` | [JuliusBrussee/caveman](https://github.com/JuliusBrussee/caveman) `skills/caveman` @ `9b68f7ed12b0` | MIT | **only** when the user types `/caveman`; `stop caveman` ends it | Added `disable-model-invocation: true`; noted that the `ultracave`/`megacave` variants are not installed. |

## Deliberately not installed

- **superpowers plugin (full)**: its SessionStart hook injects a "check for a skill before
  every response" rule, and most of its skills (brainstorming, writing-plans, code review,
  subagent-driven development) duplicate the installed Feature Dev and PR Review Toolkit
  plugins. Only the two requested skills were taken.
- **caveman plugin (full)**: auto-activates in every session through SessionStart,
  SubagentStart and UserPromptSubmit hooks, and adds about 20 extra skills, agents and a
  statusline. The always-on terse voice conflicts with CLAUDE.md §14, which asks for clear
  explanations to a beginner. The core skill is available as an opt-in instead.
- **frontend-design**: targets web front-ends; the STELLARIS UI is native C++ / Dear ImGui.
- **clangd-lsp**: needs a `clangd` binary, which is not installed on this machine.
- **Context-compression / compaction tools**: Claude Code's built-in auto-compaction is
  already active; a second compactor would be a duplicate.
