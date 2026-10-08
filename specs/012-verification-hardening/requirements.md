# 012: Verification hardening

Approved on 2026-10-08 by the explicit user instruction to implement the supplied Verification-first improvements plan. Approval covers requirements, design and T1–T4 below; package/module remain 0.7.0/0.7.

R1: Ubuntu 24.04 CI verifies Qt 6.8.0 and 6.11.3 packaging with explicit SDK selection, pinned actions, failure artifacts and a library-only build. Compatibility claims require successful runs.

R2: Independently authored complete raw native models cover IDs 148–167 and 169–191. Pin corpus checksum, validate IDs/models/review notes, record comparison method in report schema 2 and detect controlled faults. Remaining evidence limits keep strict mode failing. Production defects require separate specifications.

R3: Private decoder injection and synchronization gates verify cancelled generations, replacements, policy/base changes, destruction and subsequent success without timing races. Preserve one worker and destructor wait.

R4: Optional noninstalled benchmarks measure parsing, replacement, settled layout and sequential local image arrivals using deterministic workloads, fixed typography/viewport, two warmups and ten measurements, reporting JSON medians/maxima/counts and environment. Timings are advisory.

Excluded: public API changes, new syntax, optimization, selection/copy/accessibility, streaming, other platforms, releases and GitHub metadata.
