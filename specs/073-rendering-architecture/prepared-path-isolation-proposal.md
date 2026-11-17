# Prepared-state path-isolation experiment

## Status and approval boundary

On 2026-11-08 the owner approved **designing** this experiment and committing the
completed calibration, replying “both” to those two explicit choices. Calibration
commit `f7879b3` is complete. This document proposes new requirements PI1–PI6,
design and tasks PI-T1–PI-T6 for review. **Implementation, builds of new harnesses,
functional executable runs and measurements under this proposal are not approved.**
No next-experiment code, build or sample has been produced. Retention remains
**not established**, including if this diagnostic experiment succeeds.

The [completed calibration](evidence/candidate-scan-prepared-calibration/report.md)
resolved prepared prose reflow and original dense metadata at their diagnostic
budgets. Original prose reflow remained descriptive and failed its envelope. That
result supports drafting this proposal, not reusing calibration observations as
new controls or evidence of metric equivalence. Historical archives, tools, gates,
approvals and unsuccessful observations remain unchanged.

## Implementation approval — 2026-11-09

The owner's supplied-plan request to “Implement the plan in a fresh context”
explicitly approves PI1–PI6, this design, PI-T2–PI-T6 and the fixed 2,001-process
budget. The design-only boundary above is historical and superseded only for this
bounded experiment and evidence delivery. Stop rules and thresholds are unchanged.
Commits, push, merge, publication, production integration and further experiments
remain outside scope. Retention remains **not established**.

## Question and scope (PI1)

Does isolating the retained candidate's scratch/guard setup behind the existing
padding predicate change false-padding prepared prose reflow or original dense
metadata timings, and what residual difference remains from baseline?

Use three implementations: B, the archived ownership baseline; C, the unchanged
retained candidate; and I, a private experimental derivative of C. B/C source and
original executable identities come from the verified continuation evidence. B's
prepared executable is the unchanged size-96 refinement executable. Build matched
prepared executables for C/I and an original-contract executable for I; reuse the
unchanged original-contract B/C executables. No shipping Qt/QML library, parser,
public API, ownership algorithm or persistent cache changes enter this work.

Both measured fixtures have a false `experimentalPadded()` predicate: prose has
no spans, dense has many zero-padding spans. Verify that fact explicitly, rather
than interpreting the fixture's padding argument as proof of its execution path.
Metadata lifecycle remains the original single-instance four-operation contract.
Do not batch metadata or reinterpret prepared throughput as fresh-process latency.

## Treatment and build isolation (PI2)

Source inspection identifies a layout-local Candidate vector, guard and padding
envelope in C's wrapper before its line loop. Guards/reserve/scanning already
execute only for positive horizontal or vertical padding. The false path has no
heap scratch allocation; the proposed treatment targets state/setup/cleanup and
compiled wrapper structure, not an alleged existing allocation.

Use one explicit private dispatch at wrapper entry. On a true padding predicate,
call a private out-of-line padded wrapper containing C's candidate algorithm and
its layout-local scratch/guard state. On false, continue with the archived B
wrapper body, including its existing predicate calls and exhaustive conditional
scanner. Preserve the predicate implementation and repeated per-line calls; do
not cache the result, change span traversal or optimize metadata. The extra entry
predicate replaces C's added pre-loop predicate. Preserve empty-span return and
wrapper initialization semantics; the predicate is read-only.

The private padded helper may duplicate the baseline wrapper's common control
flow. This deliberate experimental trade-off avoids moving candidate state into
the false branch or changing the baseline's per-line predicate work. Archive a
mechanically checked source comparison: false body equals B's body, padded
algorithm equals C's algorithm modulo named function/dispatch and removed redundant
outer predicate. Preserve operation order, allocation lifetime (one vector per
wrapper invocation, capacity reused between lines), fallback resets, trimming,
legal/emergency selection, bidi, glyph/source mappings and shaping behavior.
Only TextWrapper implementation/private declarations needed for this split may
change; ScanBound.h and numerical algorithms remain byte-identical to C.

Use a compiler-supported private no-inline annotation for the padded helper, with
no LTO or unrelated flag changes. Audit the final disassembly/call graph to check
that the false entry reaches no candidate scratch construction/cleanup or guard
setup. Do not require baseline instruction identity. Code layout, dispatch and
compiler decisions also change; runtime differences cannot be attributed solely
to a particular stack initialization. If the compiler defeats the intended
isolation or the narrowly specified split cannot preserve semantics, stop before
benchmark launch and report incomplete. Do not silently substitute a different
architecture or tune flags until results look favorable.

Create fresh source/build/output/evidence directories under the sibling name
`candidate-scan-prepared-isolation`; preserve every existing workspace. Verify
historical archive/member hashes and pinned sources, binaries, headers, static
libraries, compiler, runtime libraries, fonts and fixtures before building. Reuse
verified dependency archives, rebuild only the changed SkParagraph archive for I,
and link the new C/I drivers against their respective verified libraries. Keep
Release settings and driver source identical within each measurement mode except
for library/source-root identities. Build with two workers. Diagnostic overlays
live in separate copies and never enter timed binaries. Missing/incompatible
inputs or unexplained B/C driver flag differences stop preflight.

## Fixed process budget and schedule (PI3)

Proposed budget: **2,001 functional/benchmark executable processes**, comprising
six functional checks, 75 benchmark correctness processes and 1,920 timing
processes. Compiler/linker and read-only binary-audit commands are build/preflight
operations with separate command journals, not benchmark samples. Offline Python
tests use fatal executable-launch sentinels and add no benchmark processes.
No other harness/test-executable launches, smoke runs, warm-ups, retries,
replacements or adaptive samples are permitted after approval.

First run six functional checks, in this order: C scan-tests, I scan-tests;
C numeric-tests, I numeric-tests; C fast-math-tests, I fast-math-tests. Reuse the
existing focused scanner/numerical/fallback suites in separately instrumented
copies, verifying their expected semantics against the archived implementation.
They must exercise the actual extracted padded scanner, including vertical-only
padding and late fallback, not just a duplicate helper implementation. Freeze
all six command/source identities before the first such launch. A failed test is
terminal. Reworking its implementation requires a separate amended approval; no
failed executable attempt is repeated under this protocol.

Next run 72 original-contract correctness processes: the existing twelve-fixture
implementation matrix in its archived generator order, zero then padded for each,
B then C then I. Reuse those three dense-zero correctness outputs for the dense
measurement configuration. Then run three prepared-prose correctness processes,
B then C then I, batch size 96, operation telemetry enabled. All 81 functional/
correctness results must validate before timing. Real lifecycle acceptance occurs
only in these budgeted runs; synthetic preflight launches no benchmark binary.

For each of two measurement modes, use six four-process configurations in this
fixed generator order:

| Configuration | Logical A executable | Logical B executable | Role |
| --- | --- | --- | --- |
| baseline-self | B | B | Required contemporaneous control |
| candidate-self | C | C | Required contemporaneous control |
| isolated-self | I | I | Required contemporaneous control |
| candidate-minus-baseline | B | C | Descriptive original candidate effect |
| isolated-minus-baseline | B | I | Residual effect target |
| isolated-minus-candidate | C | I | Isolation effect target |

Mode order is prepared prose (`prose-480-4096`, zero, 96, telemetry enabled), then
original dense (`dense-240-4096`, zero). Thus twelve configurations each have forty
contiguous four-process groups. Use `random.Random(73084)`: independently generate
ten shuffled `[ABBA, ABBA, BAAB, BAAB]` blocks for each configuration in generator
order; after every order sequence is complete, shuffle a fresh twelve-configuration
list for each of forty rounds. Emit all four processes contiguously. Each
configuration has twenty of each order and ten of each in each twenty-round half.
Self-control labels affect neither command nor environment; comparison labels
select only the predeclared binary, keeping fixture/options/environment identical.
Freeze the complete 2,001 identities/commands and protocol before any executable
launch, including correctness commands and software/environment fingerprints.

Use this workstation during ordinary shared use, inherited affinity/frequency,
the existing exclusive execution lock, serial launches, identical parent snapshots,
60-second process deadline and terminal 4 GiB sampled-RSS ceiling. Do not select
quiet periods, discard interference or change boost/governor/pinning. Optional
telemetry failures retain explicit reasons; required timing/resource evidence
cannot be omitted. Durable attempted-launch journals make failed/ambiguous attempts
terminal even if no child started. Resume only a validated completed prefix into
a fresh directory with unchanged frozen inputs/environment. Never repeat an attempt.

## Lifecycle and telemetry (PI4)

Prepared prose preserves the refinement harness: 96 independent instances sharing
one font collection, construct all, lay out all at 480, then time only the
increasing-index loop laying out every instance once at 497. Keep default cache
sharing and full geometry/cache/generation/mutation-isolation acceptance. Require
false padding, unchanged shaping generation and the expected state transition.
Geometry, cache checker, assertions and operation telemetry stay outside the timer;
retain the loop cost without subtraction. Compare each prepared correctness
geometry against its own original-contract prose reflow and across B/C/I.

Original-contract correctness/timing retain initial, width+17 reflow, unchanged
repeat and metadata mutation/layout. Require all exact historical geometry,
glyph/source mappings, cache events and generation signatures. Padded correctness
covers the true path even though both timing fixtures use the false path. Timing
contains no geometry/checker/diagnostic instrumentation. Do not add operation
boundary telemetry to original-contract executables. Collect identical parent
before/after observations for both modes and preserve unavailable reasons.

## Analysis and decision (PI5)

Keep the two modes and every configuration separate. For each applicable
operation, require equal paragraph counts across its four processes. Arm timings
are two-process means; `D=((B1+B2)-(A1+A2))/(2*N)`, N=96 for prepared prose or the
recorded original applicable paragraph count. Estimate median D across forty
groups. Bootstrap whole groups 10,000 times with `random.Random(73085)` reset per
configuration, shared indices across its operations; percentile indices are
`int(p*(10000-1))`. Report 95% intervals without multiple-comparison adjustment;
these are diagnostic intervals, not a familywise error guarantee or power claim.

Load exact tolerances from verified historical evidence: prepared reflow uses
4819.525000000001 and dense metadata 101542.07500000001 ns/paragraph. Values here
are for review; decisions load the archived numbers, never rounded displays.
The prose tolerance is a historical diagnostic target for a different measurement
mode, not proof of equivalence or a replacement shipping/retention gate.

Each of six self targets (three implementations × two modes) resolves only when
its entire interval is inclusively in `[-L,+L]` and its smaller median arm timing
is at least 100 times that cell's maximum clock period/empirical resolution.
Evaluate after every fixed timing process validates; do not adapt the remaining
schedule to early control results. All applicable comparison target cells must
also pass this timer rule before the following interpretation is available:

- Any integrity, correctness, build, execution, resource or evidence-analysis
  failure: **incomplete**; preserve all attempts/partial outputs and stop.
- Any self target fails: **unresolved contemporaneous controls**; report all
  completed comparisons descriptively and stop without an isolation conclusion.
- All controls resolve: report per mode whether isolation has a strict negative
  upper interval bound for I−C, and whether I−B's entire interval lies inside its
  inclusive diagnostic envelope. A mode supplies diagnostic support for reduced
  overhead within the residual target only if both conditions hold and comparison
  target timings are not limited. Report mixed outcomes explicitly. Zero-inclusive
  I−C intervals do not establish that the intervention has no effect.

C−B and all dense initial/reflow/repeat cells remain descriptive; no aggregate
speedup, retention or equivalence decision follows. Preserve nulls if encountered,
including prose metadata in the correctness matrix. Report all raw/normalized
observations, group arm means/differences, medians, unscaled MAD, range, signs,
ABBA/BAAB and first20/last20 summaries, and correctness/timing/functional resource
maxima separately. Ratios are descriptive: suppress a group if any raw time falls
below 100 times its maximum period/resolution, retain reasons/counts, never
bootstrap ratios. Do not discard required absolute timings or resource evidence.

No pooling historical/pilot/calibration observations, subtraction of control
estimates, post-hoc threshold changes or explanation of historical variability.
Even both-mode diagnostic support recommends only a separately approved decision
about any broader qualification. All other outcomes stop this iteration. No
outcome authorizes more samples, shipping integration, commits, push, merge or
publication. Retention remains not established throughout.

## Tasks and planned verification (PI6)

- [x] PI-T1 (PI1–PI6): Inspect preserved evidence and source boundaries; draft this
  concrete protocol and record design-only authorization. Run document/whitespace
  checks; record their actual results separately from these planned checks.
- [x] PI-T2: Obtain explicit approval of PI1–PI6, this design, PI-T3–PI-T6 and the
  fixed 2,001-process budget. This task is an approval prerequisite, not implied
  by authorization to design or by the earlier calibration approval.
- [x] PI-T3 (PI1–PI4): After approval, implement isolated source split, audited
  diagnostic copies, matched new drivers, frozen manifest/schedule, strict
  lifecycle/telemetry validators, supervision and terminal/prefix recovery.
- [x] PI-T4 (PI3–PI6): Before functional/benchmark launches, use fatal launch
  sentinels to test all identities/counts/order/balance/contiguity/arm mapping;
  source split/call-graph audit rejection; unchanged timer/cache/preparation;
  optional telemetry/raw-delta consistency; group normalization/bootstrap
  reproducibility/percentile indexing, zeros/offsets/drift, inclusive/strict
  boundaries, timer/ratio suppression and independent control/mode decisions;
  altered hashes, missing/duplicate/misordered records, resource failures,
  terminal/ambiguous journals, partial files and prefix/completed recovery.
  Implement forensic reports/archives for build, execution and analysis failures.
- [x] PI-T5 (PI2–PI5): Verify archives/pins, build audited targets once, freeze
  every command/input/environment, then execute the six functional, 75 correctness
  and 1,920 timing processes once. Stop on terminal failure; never retry a slot.
- [x] PI-T6 (PI5–PI6): Require byte-identical reanalysis and completed-run resume
  with zero launches under fatal sentinels. Run focused and repository Light
  checks/whitespace validation. Archive new tooling, changed-source patches and
  source copies, reused/new binary bytes, libraries/recipes or verified historical
  references, frozen context, audits, raw outputs/telemetry, attempted journals,
  analyses/decision and check logs. Read back every archive member hash; revalidate
  preservation of history. Update actual verification/recommendation/tasks,
  including incomplete results. Stop after evidence delivery.

## Execution outcome — 2026-11-09

The approved 2,001 processes completed once: six functional, 75 correctness and
1,920 timing processes. All six contemporaneous controls resolved. The result is
**mixed diagnostic support**: original dense metadata meets both diagnostic
criteria; prepared prose has a zero-inclusive I−C interval and does not establish
an improvement. See the [actual report](evidence/candidate-scan-prepared-isolation/report.md)
and [verification record](verification.md). No threshold, budget or stop rule changed.
Stop this iteration; retention remains **not established**.
