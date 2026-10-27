# Iteration 069 — Streaming Markdown

Approved scope: the user explicitly requested implementation of the supplied Iteration 069 plan on 2026-10-27. This approves requirements, design, tasks and local verification only. Changes remain uncommitted; publication and installed-package replacement are excluded.

Opt-in streaming accepts append calls and strict suffix-growing assignments. Accepted source notifies immediately; full parsing publishes after a non-restarting 16 ms interval or synchronous flush. Disabling flushes; replacement cancels batching and publishes immediately. Settings consume pending source under current settings. Each publication has static semantics, including incomplete syntax and late definitions.

Retain unchanged recursive models, delegates, actions, resources and reading state. Preserve only unchanged semantic selections and surviving link/Find occurrences. Never scroll automatically. Preserve existing security and resource limits. Prepare package 1.18.0 development / QML 1.18 and preserve published 1.17.1 documentation.

## Subsequent release delivery approval — 2026-10-27 UTC

The user explicitly approved the supplied “Release QMarkdown 1.18.0 / QML 1.18” plan and requested implementation. This extends the original local-only authorization to commit, push, open one release PR, protected squash merge and stable/latest publication. Historical local scope and evidence above remain unchanged. Host-installed package replacement remains excluded.

Retain the implemented streaming feature, tests, playground and documentation together, plus changes merged since 1.17.1. Add no features; permit contract fixes. Keep package 1.18.0, QML 1.18, SOVERSION 1 and dependencies. Finalize dated release notes, changelog and all active release/install/compatibility documentation using 2026-10-27 UTC. Describe full snapshot parsing, retained state, binding ownership and host scrolling, with bounded measurement claims.

Run task check, git diff --check and scripts/check-docs.py --release. Existing local Full evidence supports review; the latest final-head hosted Full matrix on exact Qt 6.8.0/6.12.0 and retained qmarkdown-ci-v2 gate authorize publication. Open “Release QMarkdown 1.18.0”; after passing protections squash merge, keep main fixed, dispatch release.yml with the PR number, and require verified-tree, package and Qt identity gates. Verify extracted source builds with syntax OFF/ON on both targets and focused Arch checks. Publish source/Arch archives and their SHA-256 companions under v1.18.0 stable/latest; completion requires all four downloaded assets matching checked bytes. Stop on failure, moved main or differing assets; never move tags or overwrite differing bytes. Keep delivery evidence in the PR/Actions without an evidence-only follow-up PR or upkeep commit.
