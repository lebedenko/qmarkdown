# Design

Approved on 2026-10-09 by the explicit instruction to implement Iteration 017. Approval covers requirements, design, tasks and verification below; production changes require renewed approval.

Keep the production parser untouched. Store complete source-reviewed expectations in a new schema 1 fixture pinned to the unchanged upstream checksum and a code-owned exact ID set. Review Markdown alongside CommonMark 0.31.2 rules; official HTML may corroborate structure but cannot supply opaque source fields. No probe output, bundled parser trees or fingerprints author expectations.

Each model result contains required checks: source only for the existing 69, HTML only for 193, and HTML plus complete source for 390. Aggregate failure retains mismatch evidence in checks; complete passing source comparison resolves aggregate limitations while leaving HTML annotations visible. Report validation checks required methods and aggregation.

CodeBlock retains recognition, content and full decoded info; fenced versus indented syntax origin is outside the retained contract. No new production metadata. Normalization merges adjacent Text alone and preserves empty children, containers, break identity, UTF-16 offsets and exact strings.
