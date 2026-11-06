# Candidate-scan runtime evidence — terminal correctness failure

Owner approval: supplied-plan implementation request, 2026-11-06, recorded before
coding in CR1–CR5 and CR-T1–CR-T5. **Retention is not established.** No timing
processes ran, so neither measured benefit nor a measured runtime regression is
established. All performance gates remain unevaluated due to incomplete correctness
and sampling. The [report](report.md) and [summary](summary.json) list all 176
fixture/padding/operation cells and their twenty missing pairs; there is no pooled
speedup or invented confidence interval.

## Actual execution

- Persisted schedule: 88 correctness then 1,760 timing processes; twenty adjacent
  baseline/retained pairs per cell, alternating order each pass. Twelve fixtures
  remain byte-identical. Ten small fixtures have 32 independently constructed
  deterministic paragraphs at 64/256 UTF-16 units, width 480.
- Two fresh ownership-baseline copies were cleaned and rebuilt through 938 Release
  build steps each with the same pinned dependencies/settings and driver. Only
  retained receives the verified clean candidate patch. Source/flag/library and
  binary-symbol audits passed before any correctness process. Neither arm contains
  phase counters, diagnostic selectors or shadow evaluation. Affinity, host load,
  tools, dependencies and per-process C/MXCSR state are retained. No system-setting
  changes or independently cold-cache claims.
- The first 48 processes (twelve historical fixtures × zero/padded × both arms)
  completed all four operations: 192 operation records match exact paired geometry,
  glyph positions, visual slices, mappings and shaping deltas, as well as
  hash-validated archived exhaustive traces.
- Process 49, `correctness-00-small-prose-480-64-zero-baseline`, exited 2 with
  `shaping invariant failed`. Its stdout is empty; stderr, exit status, command,
  fixture/raw hashes and terminal runner traceback are retained. Thirty-nine
  correctness processes and all 1,760 timing processes remain unlaunched. The
  failure is terminal. There was no retry, fixture/threshold tuning, extra sampling
  or substitute optimization.
- Static inspection identifies an invalid inherited driver assumption: initial
  layout requires generation delta 1 for every paragraph. Identical metadata-free
  paragraphs share a FontCollection; ParagraphImpl can restore from ParagraphCache
  without calling shapeTextIntoEndlessLine (which increments the generation).
  Independent construction does not disable that cache. The aborted output does
  not preserve the paragraph/delta that triggered the assertion; this is a source
  diagnosis, not an additional experimental trace or proof of cutoff regression.

## Verification and limits

Archived cutoff/proof members and archive hashes were read back; live cutoff
identities, clean patch and completed proof checkpoint match their archived hashes.
Unchanged numeric/fallback proof suites were not rerun. Historical boundary/extent
verification ran independently after the terminal outcome, without adding runtime
samples: 751 disabled-equivalence fixtures / 775 blocks, 775 scan blocks, 14,272
vertical cells and 2,901 visual fragments pass. Containment tolerance remains 0.05,
with the same 95 near-edge snap observations.

The new harness has ten passing tests covering schedule/alternation, fixtures,
operation applicability, timer suppression, paired statistics, exact-output
rejection, threshold boundaries, missing samples and terminal failure. Existing
runtime/recovery/supervisor/retained/ownership tooling totals 40 passes and one
historical optional attribution skip. Repository lightweight and whitespace checks
are retained separately. A preparation-time Python import-path collision was fixed
before builds/sampling; no runtime driver or fixture was changed after the failure.

Fatal sentinels on all process launch routes verify that resume rejects this
terminal study without launching a process or creating a destination. A
**completed-run** resume and complete-study validator cannot run because no
completed run exists; they are not claimed as passed. Build, execution and gate
journals are separate and hash-sealed. Reporting preserves incomplete cells and
null/missing distinctions. The declared 10,000 whole-pair bootstrap (seed 73075)
produces no estimates without timing pairs.

Maximum sampled correctness RSS is 50,614,272
bytes; maximum correctness high-water RSS is 133,435,392 bytes. These include geometry
and whole-process state and are **not timing-memory comparisons**. The existing
verified scratch payload bound remains 524,320 bytes (16,385 × 32-byte entries),
excluding vector header/allocator overhead, with zero scratch on the zero-padding
path. There is no separate preprocessing/allocation cost attribution.

[archives.json](archives.json) inventories sources, clean patch, configuration,
identities, environment, commands, fixtures, raw successful/failed outputs,
traceback, reports, gates and tests. Every archived member and archive hash is read
back and verified. Previous work and archives remain unchanged. Shipping Qt
integration, ownership changes, further optimization, merge and publication remain
excluded. Any future study requires a separately approved response to the driver’s
cache assumption and a new predeclared schedule; this run remains terminal.
