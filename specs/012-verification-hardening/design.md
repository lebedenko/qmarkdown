# Design

Approved on 2026-10-08 by the explicit user instruction to implement the supplied Verification-first improvements plan. Approval covers requirements, design and T1–T4 below; package/module remain 0.7.0/0.7.

T1 uses install-qt-action provisioning only in CI; packaging accepts an explicit SDK and artifact directory and passes Qt6_DIR to every configuration.

T2 stores literal authored models and per-ID CommonMark boundary rationale in a separate fixture. Source-authored cases compare raw models; all other cases retain the HTML projection. No production output creates expectations. Remove only individually superseded HTML-oracle ledger exceptions.

T3 copies a std::function decoder into each worker. Tests gate entry/completion around the real decoder with bounded semaphore waits and RAII release. Active destructor tests release from a helper thread because destruction waits for the worker; this can block the calling thread until decoding ends. No scheduler redesign.

T4 builds a standalone executable only when requested, requiring BUILD_TESTING and Qt Test. Parse and model measurements exclude QML; layout uses a 480-pixel view and fixed Noto Sans/Noto Sans Mono fonts, event processing and explicit polish until stable. Image arrivals use a private controller with gated real local decoding and the production projection/reset path. Benchmarks are absent from CTest and install exports.
