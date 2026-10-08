# Requirements

Approved on 2026-10-08 by the user’s explicit instruction to implement the supplied Feature 013 plan. Approval covers these requirements, design, tasks and format preparation only; package/module remain 0.7.0/0.7.

- R1: Replace quadratic interval preparation with O(S log S) time and O(S) temporary storage, decoding each QVariant span once.
- R2: Preserve UTF-16 offsets and every valid span boundary, without adjacent merging. Accept only positive in-bounds ranges with overflow-safe validation; ignore invalid ranges. Emit intervals only for supported emphasis/strong/code flags or active links. Overlapping duplicates stay active until final coverage ends.
- R3: Preserve native rendering, font resolution/rounding, link styling, wrapping, raster bounds, painting, public API and versions. No dependencies, parser/image changes, caching, virtualization or link hit-testing optimization.
- R4: Demonstrate output equivalence with an independent character oracle and test-only previous algorithm. Add opt-in noninstalled preparation benchmarks (1,024–8,192 formatting spans, with/without overlapping links), two warmups and ten samples, validated JSON and no CI timing thresholds. On the same Release environment require lower dense-layout median and preparation scaling consistent with the new algorithm.
- R5: Run focused tests, shared/static CTest, QML lint, CommonMark baseline, symbol privacy and benchmark installation exclusion checks. Separate local results from pending Qt CI and full conformance claims.
