# Requirements — Optional source-line gutter

Approval: approved on 2026-10-16 by the user instruction to implement the supplied “Publish QMarkdown 1.11, then implement 1.12 code-block usability” plan. Scope includes Features 050–053 and one package 1.12.0 / QML 1.12 release PR. Merge, tagging and publication of 1.12 require separate authorization.

Expose codeBlockLineNumbersVisible default false and resettable style.codeBlock.lineNumberColor with effective foreground alpha multiplied by 0.6. Number physical source lines from 1 per block, blank continuation rows, terminal newline omission and empty line 1. Fixed physical-left gutter uses code font and right-aligned digits, 8px padding per side. Exclude numbers from copy, search, selection and accessibility.

## Requirement map

- **R053.1:** Physical source lines, blank lines, terminal omission and shared baselines.
- **R053.2:** Decorative left gutter, visible numbers only and source-only interaction.
- **R053.3:** Effective font/color reset, widest-number sizing and no retokenization.
