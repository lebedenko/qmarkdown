# Design — custom fenced-block renderers

Status: **approved on 2026-10-14** by the user response “approved”. Requirements
R1–R8, design and tasks T1–T5 are authorized for implementation and verification.
Package 1.7.0 / QML 1.7 is retained; no version bump, tag or publication is authorized.
See [requirements](requirements.md), [design](design.md), [tasks](tasks.md) and
[verification](verification.md).

## Dispatch and private metadata

Expose a QML `var` map named `fencedBlockRenderers`, initially empty, on MarkdownView.
Hosts assign a new map to update dispatch, for example `{ "preview": previewComponent }`.
Do not observe nested mutations. Look up only own map entries so inherited object
properties cannot become renderers. Tokenize decoded info using whitespace, skipping
leading whitespace; retain the complete decoded string separately. Matching is exact
and case-sensitive, with no aliases or normalization. Empty tokens bypass dispatch.

The semantic Block currently retains decoded info but does not retain fence origin;
PresentationBlock currently carries neither. Add private origin metadata from the
parser and copy origin/full info through presentation projection and model roles to
recursive code delegates. Derive language from that decoded info, never from raw
Markdown or code content. Keep code text unmodified; the ordinary display path may
retain its existing display-only terminal-newline handling. Extend private fixtures
without changing the pinned CommonMark semantic contract or exposing parser types.

## Component contract and content ownership

Use the existing private CodeSurface as the owner of the content instance. Preserve
CodeActionHost, surface decorations and padding. For a matched component, create its
Item with initial `codeText`, `infoString`, `language` and `markdownStyle` values so
required properties are satisfied during construction. Proposed host declarations:

```qml
required property string codeText
required property string infoString
required property string language
required property var markdownStyle
```

The supplied style is the view's effective MarkdownStyle object. Propagate style
replacement, and allow bindings to receive live changes to that object's properties.
Assign the available content width on creation and resizing; the host owns internal
layout. Use `max(0, implicitHeight)` for finite heights, and zero for nonfinite values.
Reevaluate reserved height when width or implicitHeight changes. The existing surface
accounts separately for action header height and padding. Do not use the custom
Item's explicit height as the layout contract or impose a minimum text row on it.

Validate Component readiness and use a private helper to validate the Item result
and writable string/property contract. Qt initializes required properties during
construction; its public metaobject API does not reliably retain QML required
declaration flags. The host must declare the properties required, but the library
does not inspect those flags or depend on private Qt APIs. Reject null/invalid entries or unsuitable
roots; destroy partial instances and issue a QMarkdown diagnostic containing the
language and failure reason without dumping source. Fall back to ordinary code.
Avoid retry loops: retry after relevant document/map/component or selection changes.

Destroy the previous instance before replacing its document/component or removing
the delegate. Reassignment that changes the selected component refreshes the content;
width/style changes update the existing instance. Document replacement creates fresh
instances even when text compares equal. Use the same path for nested lists/quotes.

## Selection, actions and trust

`selectionEnabled` chooses the ordinary selectable-source path before custom dispatch.
Enabling it destroys custom instances; disabling it recreates eligible renderers.
Keep the existing source projection, selection offsets and action payloads independent
of renderer output. Action headers remain available and copy parser-produced code.
No custom visual text participates in document selection.

Hosts supply trusted application Components and may use their application's existing
capabilities. The library neither sandboxes them nor adds engines, network requests
or permissions. Keeping only content replaceable preserves existing styling/actions
and avoids introducing a second surface/layout API. Selection deliberately displays
source because arbitrary host Items cannot provide the existing text geometry contract.

## Example and delivery

Plan a playground `preview` renderer made only from native QtQuick Items/Text/shapes,
with all four required properties, width-dependent implicitHeight and style bindings.
Demonstrate whole-map reassignment, unknown-language fallback and selection toggling.
Add a standalone installed consumer exercising the same public contract through
shared, plugin-only and static imports and relocation. Document the API only when
implemented; this draft is not a current public API reference. Follow the
[development workflow](../../docs/development-workflow.md) for one meaningful change,
required PR gates and separately authorized publication.
