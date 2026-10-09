# Design

Approved on 2026-10-09 by explicit user approval. Scope covers requirements, design and tasks for the responsive decoder lifecycle and its verification.

## Ownership and scheduling

Introduce private decode scheduler/job types beside ResourceController, included in existing library/test build integration. Lazily create one scheduler owned by QCoreApplication on the application thread. Controllers borrow it and own only their current shared job handle and watcher. The scheduler owns a dedicated two-worker QThreadPool, a mutex-protected FIFO pending queue, active-slot accounting and a stopping flag. Submission/cancellation occurs on the application thread; worker completion releases capacity and dispatches the oldest eligible request under synchronization, without depending on an engine or controller event loop. Never execute a decoder while holding the scheduler mutex.

Keep jobs out of the pool's internal pending queue: dispatch only up to the scheduler's two active slots. Each job holds copied URL, encoded bytes, decoder callable, monotonic admission deadline, atomic cancellation state and shared promise/result state. Workers capture only that job state and scheduler state whose lifetime is protected by shutdown draining. Private fixture seams may inspect scheduling or shorten a deadline without changing production's 15 seconds or public APIs.

Cancellation marks the job cancelled, removes it from the pending queue immediately, releases retained input/result storage where safe and finishes a never-dispatched promise exactly once. A running job owns its input until completion. Recheck cancellation and deadline before local reading, before decoding and after decoding; discard a stale image before reporting completion. Each current controller submits one decode at a time; replacements cancel their predecessor first. Active stale jobs count toward the global two-worker cap but no longer prevent a replacement from submitting.

## Controller completion and deadlines

Replace m_decoder with scheduler submission/cancellation and a shared job handle. Destructor cancels the job, deletes the watcher and aborts/disconnects networking without waiting. Workers never capture this or consult policies. Preserve the existing controller generation counter and sequential document pump.

Before reading a future result, check current generation/job identity, cancellation and result availability. Handle empty cancelled/expired completion as a failed admission and advance the current document safely. Recheck deadline and retained budget before publication; retain the generation check after changed(), because signal handlers can replace the document. Controller timer expiration cancels any matching decode job as well as network work; queued expiry must finish without invoking its decoder. Preserve existing fetch concurrency, URL authorization and cache invalidation.

## Shutdown and compatibility

Scheduler destruction first stops admission and cancels/finishes all pending jobs under synchronization, then waits for active workers outside the mutex. Only after draining may scheduler state be destroyed. This must not require queued GUI-thread callbacks to finish. QThreadPool destruction also waits for running work; see the [Qt documentation](https://doc.qt.io/qt-6/qthreadpool.html#dtor.QThreadPool). Codec interruption and library unloading during active work remain outside scope.

Use Qt 6.8 facilities and existing C++ conventions. No public type, dependency or version change. The trade-off is a global two-decode bound: one stale job leaves capacity for fresh work, while two held codecs necessarily occupy both slots.

## Evidence

Extend the existing Qt Test resource and native view fixtures with synchronization gates and bounded failure cleanup. Prove teardown returns before gate release and safe completion after destruction. Use a separate process for application teardown so its drain behavior is observable without destroying the main test application's scheduler. Verify FIFO at dispatch rather than asynchronous completion order. Keep historical Feature 019 results intact; amend its current coverage assessment with links to this iteration's actual evidence after implementation.
