# Specifications

Repository-owned specifications are the source of agreed scope. The initial design input was `/tmp/qt-markdown.md`; that temporary brief is not required to read or maintain these documents.

- [Project overview](overview.md): accepted architectural direction and unresolved project decisions.
- [Standards and conformance](standards.md): pinned CommonMark release gate, selected extensions, and future verification.
- [Roadmap](roadmap.md): staged intentions, not delivery commitments.
- [001: static text requirements](001-static-text/requirements.md)
- [001: design](001-static-text/design.md)
- [001: tasks](001-static-text/tasks.md)
- [001: verification](001-static-text/verification.md)
- [002: project scaffold requirements](002-project-scaffold/requirements.md)
- [002: design](002-project-scaffold/design.md)
- [002: tasks](002-project-scaffold/tasks.md)
- [002: verification](002-project-scaffold/verification.md)
- [003: inline formatting requirements (implemented and verified)](003-inline-formatting/requirements.md)
- [003: design](003-inline-formatting/design.md)
- [003: tasks](003-inline-formatting/tasks.md)
- [003: verification](003-inline-formatting/verification.md)

Each numbered feature contains these four records. Requirements define observable behavior; design describes how to deliver it; tasks trace the work to requirements; verification distinguishes planned checks from recorded results. Feature 001 requirements, design, and tasks were approved on 2026-10-07 and its bounded renderer is implemented; feature 002 scaffolding was approved on 2026-10-07. Feature 003 was approved on 2026-10-07 and implements the bounded inline-formatting slice in version 0.2.0. Future features must repeat the approval cycle defined in [AGENTS.md](../AGENTS.md).

- [004: fenced code block requirements (implemented and verified)](004-fenced-code-blocks/requirements.md)
- [004: design](004-fenced-code-blocks/design.md)
- [004: tasks](004-fenced-code-blocks/tasks.md)
- [004: verification](004-fenced-code-blocks/verification.md)

- [005: demo playground requirements](005-demo-playground/requirements.md)
- [005: design](005-demo-playground/design.md)
- [005: tasks](005-demo-playground/tasks.md)
- [005: verification](005-demo-playground/verification.md)
