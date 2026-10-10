# Requirements

Approved on 2026-10-10 by the explicit instruction to implement the supplied release plan, including PR merge, tag creation and stable GitHub publication.

- R1: Align compatibility documentation and prepare dated 1.0.0 notes; retain historical candidate evidence.
- R2: Require both documentation PR packaging checks and a manual stable-package run on the final main commit, including both build and both binary-consumer jobs. Review artifacts before tagging.
- R3: Publish v1.0.0 at that commit, require upload workflow success, verify downloaded asset checksum and disposable Ubuntu installation/removal. Record publication evidence.
- R4: No API or feature changes; retain Linux amd64 source and Ubuntu 24.04 x86_64 binary scope and accepted limitations.
