# Fixed prepared-state batching pilot evidence

The owner's supplied-plan request approved this isolated harness and one fixed
297-process pilot on 2026-11-08. All 297 processes completed validly: nine correctness
and 288 timing, with no smoke runs, warm-ups, retries or replacements. No renderer
or public API changed. No commit, merge, publication or later calibration ran.

**No size selected; retention remains not established.** Enabled batch 64's median
was 9.521602 ms, below the inclusive 10 ms target. Enabled batch 256's maximum sampled
RSS was 1,127,469,056 bytes, above the strict 1 GiB selection ceiling. Enabled batches
1 and 16 were also below 10 ms; batch 1 additionally failed MAD/median <= 10%.
No threshold or budget was changed after observing results.

- [Report](report.md): configuration summaries and interpretation limits.
- [Analysis](analysis.json): raw/normalized timing rows, all groups, A/B means and
  differences, signs, MAD/ranges, order/half summaries and matched-round telemetry.
- [Selection](selection.json): deterministic completed/no-selection result.
- [Archive manifest](archives.json): SHA-256 for the archive and every member.
- [Read-back log](archive-verification.log): 631 members verified and historical
  pinned identities revalidated after packaging.

The archive contains the new source, build and check logs, source/compiler/binary/
fixture/font/library identities, frozen seed-73080 schedule and environment,
all raw results/stderr, parent snapshots, operation-boundary telemetry, durable
attempt/index journals, protocol, analysis, selection and specification snapshot.
Historical archives are referenced by verified identity rather than duplicated.
Original-reference, correctness and prepared-state results remain distinct.

Ten focused tests passed under fatal launch sentinels. Repository lightweight and
whitespace checks passed. Completed-run resume and full reanalysis revalidated all
297 records with **zero launches**, producing byte-identical `analysis.json`,
`selection.json` and `report.md`. All nine declared correctness processes passed
geometry, shaping/cache and applicable instance-isolation checks. The Release
harness used two workers and compile definitions/flags matching the unchanged
baseline libraries. Every archived member hash was read back successfully.

Working outputs remain in `build-ci/073-candidate-batching-pilot`; completed resume
is `build-ci/073-candidate-batching-pilot-resume`. Sources and reproduction entry
points are in [the sibling tool](../../demonstrator/candidate-scan-batching-pilot/README.md).
Any later calibration requires separately approved requirements, design, tasks and
budget. This pilot cannot qualify historical results or become that dataset.
