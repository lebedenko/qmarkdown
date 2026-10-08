# 011: CommonMark conformance baseline requirements

**Approval:** On 2026-10-08 the user explicitly approved these requirements, design and tasks. Scope: T1–T6, test infrastructure and documentation only; package/module 0.7.0/0.7 and production behavior remain unchanged. Production fixes require subsequent approval.

## Purpose

Feature 010 completes the currently approved image/resource slice. Establish reproducible evidence for the existing CommonMark 0.31.2 parser and native document projection before extending syntax. This iteration establishes a baseline; it does not promise the v1.0 conformance gate will pass.

## Requirements

- R1: Vendor every official CommonMark 0.31.2 example with its identifier, section, Markdown and expected HTML unchanged. Record acquisition URL, SHA-256, version, attribution and applicable fixture license separately from authored MIT code. Builds and tests must work offline.
- R2: Exercise the repository's privately bundled cmark with production parsing options against every official example. A test-only HTML serializer may compare exact upstream expectations. Count every example once and report pass/fail with identifiers, section totals and readable expected/actual differences. Do not silently skip or normalize away mismatches.
- R3: Exercise the production `QMarkdownPrivate::parse` path separately. Compare native model output with expectations derived independently from official HTML wherever the model has an equivalent representation. Cover block order/kinds, recursive containers, heading levels, list start/tightness, text, formatting, hard/soft breaks, link destinations and image metadata. Account explicitly for every example, including unsupported oracle constructs and distinctions the model cannot preserve.
- R4: Document intentional presentation differences and information loss without treating either as full semantic conformance. Examples include literal HTML, space-projected soft breaks, image-description formatting, omitted link titles, collapsed emphasis nesting, and invisible empty links. Independently verify authored edge cases for UTF-16 ranges and zero-length/adjacent images. Parser passes cannot stand in for model checks or native rendering evidence.
- R5: Provide one offline command that runs both layers and emits a deterministic machine-readable report plus a concise summary. Ordinary CTest must reject unexpected regressions, missing/duplicate fixtures, invalid reports and newly unaccounted examples. An explicitly selected strict mode must fail on any parser mismatch or incomplete model semantic coverage; expected failures remain visible in totals.
- R6: Record actual counts and commands, classify discovered gaps, and recommend a bounded follow-up with affected example identifiers. Preserve package/module 0.7.0/0.7, public API, production behavior, resource policy, dependency privacy and installation contents.

## Excluded scope

No parser upgrade, production semantic/model fixes, new public API, GFM syntax, selection/copy, accessibility, streaming, renderer extensions, inline image composition or v1.0 declaration. Findings needing behavior changes require a subsequent approved specification. This is test infrastructure and evidence, not a release commitment.
