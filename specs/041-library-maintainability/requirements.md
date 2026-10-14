# Requirements — library maintainability

Status: approved on 2026-10-14 by the user response “approved”. Requirements,
design and tasks T1–T7 are authorized for local implementation and verification.

## Scope

Address the five review findings and the associated ownership, header dependency
and public-root helper observations. Preserve package 1.7.0 / QML 1.7 and all
documented public behavior. This is one meaningful maintenance change.

- R1: Replace unnamed model roles, rendering kinds and selection-item kinds with
  named private contracts shared with QML where necessary. Preserve existing role
  names and values, including selected/unselected rendering behavior.
- R2: Separate cohesive presentation responsibilities and remove duplicate
  code-block surface/action geometry. Preserve selected and unselected text paths,
  recursive containers, table layout, action lifetime and gesture routing.
- R3: Separate parser-owned semantic blocks from image/layout/selection projection
  data. Parsing must not construct decoded images or synthetic presentation blocks.
  Preserve semantic fixtures, copy text, offsets and resource replacement behavior.
- R4: Replace string-keyed style storage and dynamically constructed notification
  names with typed state and explicit notification dispatch. Preserve all defaults,
  presets, explicit overrides, legacy/grouped aliases, font resolve masks, reset
  behavior and dependent table foreground notifications.
- R5: Provide one authoritative public QML/CMake API reference with defaults,
  versions, ownership, reset rules, signals, methods and action component contracts.
  Correct contradictory current capability statements. Keep historical evidence.
- R6: Keep implementation helpers off the public MarkdownView root. Preserve all
  documented public properties, signals and methods; do not expose new C++ APIs.
- R7: Make parser-tree ownership noncopyable, document network-manager ownership
  transfer, express mandatory policy input as a reference, and reduce unnecessary
  header dependencies without breaking Qt metadata generation.
- R8: Preserve shared/static packaging, installed/relocated consumers, source archive
  completeness, parser symbol privacy and the configured Qt compatibility matrix.

## Exclusions

No new syntax, streaming, incremental rendering, public parser/AST, renderer plugin
API, new dependency, package/version bump, tagging or publication. Existing raw
QObject pointers with parent ownership and shared decoder-job ownership remain
valid patterns; this is not a blanket smart-pointer or PIMPL conversion.
