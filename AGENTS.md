# Project instructions

## Communication and work style

- Communicate in English, including plans, recaps, status updates, tool commentary, and final answers. Generate Russian or Ukrainian only when explicitly requested in the current message; never infer language from locale or context.
- Prefer small, reviewable changes. Inspect repository structure, specifications, and existing conventions before editing.
- Preserve existing style. Use simple, maintainable code; introduce libraries, frameworks, or large abstractions only with clear justification. Explain architectural trade-offs.
- Use one PR per meaningful change and squash merge by default. Keep implementation, tests, documentation, and verification together.
- Follow [development workflow](docs/development-workflow.md) for current delivery guidance; it supersedes historical delivery procedures while preserving historical approvals and verification records.

## Safety and verification

- Never run destructive commands without explicit confirmation or rewrite history unless explicitly requested.
- Do not touch secrets, credentials, `.env`, private keys, or generated dependency lockfiles unless required.
- Run the narrowest relevant checks first. If checks cannot run, explain exactly why. Report changed files and verification before finishing.
- Repeat broader checks when inputs change, checks fail, or concerns remain unresolved, not merely to record previous results. Use lightweight checks for prose-only changes and full relevant verification for build-affecting changes; existing CI triggers and required checks still apply.

## Spec-driven development

The cycle is **requirements → design → tasks → user approval → implementation → verification**.

- Maintain repository-owned specifications under [specs/](specs/README.md). Number feature directories and keep requirements, design, tasks, and verification records together.
- Distinguish accepted project direction, draft feature requirements, illustrative APIs, and future possibilities. External briefs are design input, not automatic commitments.
- Before feature implementation, resolve implementation-blocking decisions in the design, map requirements to tasks and checks, and obtain explicit user approval of the requirements, design, and tasks. Record approval and its scope in the feature documents.
- Changes to approved scope require updated documents and renewed approval before implementing the changed scope. Routine fixes within approved scope may proceed.
- Record actual verification results separately from planned checks. Never imply that a planned feature, test, or viewer already exists.
- Treat integration, evidence recording, and publication as delivery tasks within the meaningful change; do not automatically create separate numbered initiatives for them.

Feature 002 project scaffolding was explicitly approved on 2026-10-07; its build, module packaging, import-only example, and packaging checks are authorized. Feature 001 requirements, design, and tasks were explicitly approved on 2026-10-07 via the implementation plan; the bounded static-text slice is authorized.

## Architectural boundaries

The product is an independent Qt/QML library: Markdown source → private parser → document/AST model → native Qt Quick block components. Host applications own visual identity. Do not introduce HoloNight dependencies or document-level HTML, QTextDocument, or WebEngine rendering. Resource policies, streaming, and fenced-block extensions are planned capabilities, not first-slice implementation requirements.
