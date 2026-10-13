# Design

Approved with requirements and tasks on 2026-10-13 through the supplied plan.

Retain `packaging (minimum)` and `packaging (development)` contexts. PR scope
classification enables full verification for non-prose inputs and release
notes/changelog changes. Both required jobs validate current documentation even
for prose-only PRs. Complete runs retain a small `release-gate` Actions artifact
containing final PR head, tested merge tree, exact Qt configuration and package
version. It is never a release asset. Expired/missing gate evidence fails closed.

Manual publication takes a merged PR number, finds its latest final-head PR run,
and compares the gate against merged/main inputs. Source distribution is created
from committed allowlisted files with normalized tar/gzip metadata. The same
CMake entry point defaults to library-only everywhere; repository development
commands explicitly enable tests/examples. The archive renames standalone
instructions to INSTALL.md. Focused extracted builds/installations use both
configured Qt SDKs. Arch consumes identical source bytes and separate development
consumer tooling; makepkg builds without CTest. Disposable containers check
metadata, dependencies, payload, ownership, consumers and removal once.

Publication rechecks eligibility, validates exactly four local assets, creates
an unmoved version tag and draft release, uploads only absent identical-compatible
assets, rechecks eligibility, publishes stable/latest and hashes downloaded bytes.
Retry refreshes asset state after partial uploads; differing remote assets fail.
A failure reports its stage and leaves tag/release state for an explicit repair.
Execution logs stay in Actions. No automatic cleanup or history rewriting occurs.

Historical feature documents remain immutable evidence of earlier checks and
approvals; current summary documents describe released 1.6.0. Existing Ubuntu
instructions move to a clearly historical page. No runtime version/API changes.
