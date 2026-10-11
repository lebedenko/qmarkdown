# Verification

## Planned

All checks mapped in tasks.md and requirements.md. Both configured container targets follow completed CI preparation. Release publication is excluded.

## Actual — in progress, 2026-10-11

- Package/module versions are 1.1.0/1.1. Grouped state, presets, explicit overrides, aliases, binding-preserving resets, native heading/code/inline decoration, playground controls and installed 1.1 consumers are implemented.
- Focused host Qt 6.12.0 checks pass for style state/fonts/declaration order/bindings/RESET/revisions/shared-style destruction, heading levels/containers/image transitions, literal/empty/narrow/invalid code geometry and inline wrap/bidi/combining/emoji/emphasis/link geometry and hit testing. New styling suites pass at scaling 1 and 1.25 and save all five preset fixtures in build-shared/tests/presentation.
- Host shared/static packaging checks pass in build-packaging-025-verified, including CTest 10/10 in both variants, QML lint, symbol privacy, package requests, installed/relocated 1.0 and 1.1 linked/plugin consumers and library-only build. Strict CommonMark checks pass all 652 parser/model examples, with zero mismatches/limits/losses.
- Initial new tests incorrectly assumed monospace always changed the SDK font and sampled a glyph pixel in unpadded Legacy code. Assertions now force an actual font change and restrict interior-background sampling to padded Polished surfaces, retaining exterior/background/border geometry checks without exact glyph comparisons. Existing fence/consumer tests were adapted to native code surfaces rather than depending on parent structure.
- Both complete container suites are running with corrected tests. Final results and immutable identities remain pending. No release was published.
