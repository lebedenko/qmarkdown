# Requirements

Approved on 2026-10-09 by explicit user approval. Scope covers requirements, design and tasks for the responsive decoder lifecycle and its verification.

## Purpose

Make MarkdownView and QML engine destruction independent of active image decoding, while allowing fresh work to proceed beside one cancelled decode.

- R1: Controller, view and engine teardown must return while a decoder remains held. Detached workers must never access controllers, views, engines or resource policies; stale work cannot publish.
- R2: Use a private application-owned scheduler with a dedicated QThreadPool capped at two workers. A fresh generation or another controller can complete beside one stale active decode. Two occupied workers may delay fresh work; running codecs cannot be interrupted.
- R3: Dispatch pending requests FIFO. Keep at most one current request per controller; cancel and remove queued requests immediately. Repeated replacement must not accumulate obsolete jobs. Cancelling one controller must not cancel another.
- R4: Preserve sequential image admission within each current document, per-view networking/policies/caches, resource limits and network cancellation. Retain the 15-second admission deadline, including scheduler waiting. Expired queued requests must finish without decoding; expired results cannot publish.
- R5: Application teardown stops admission, cancels queued jobs and drains active workers before scheduler state is destroyed. Application shutdown may wait for codecs.
- R6: Preserve public APIs, dependencies, package/module versions 0.7.0/0.7 and Qt 6.8 compatibility. Update current lifecycle documentation and native coverage after verification; retain historical evidence.
- R7: Record focused resource/view checks, strict CommonMark verification, shared/static CTest and both pinned local CI tasks, including actual counts, commands, artifacts and limitations.

Excluded: new syntax, rendering optimizations, process isolation, release publication and decoder-library unloading while work remains active.
