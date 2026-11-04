# Stable streamed code blocks

Approved on 2026-11-04 through the user-supplied implementation plan and explicit request to implement it. Approval covers requirements, design, mapped tasks, implementation and verification. Merge, publication and system-package replacement require separate authorization.

## Design

ViewState signals the retained publication classification immediately before model mutation. Each surviving code surface captures its old source anchor and bottom proximity at that boundary. Suffix growth consumes this context, queues generation-guarded restoration and either follows the settled bottom or restores the anchor. Geometry restoration retains follow intent; observed viewport movement updates the next append position. Model reset invalidates queued restoration.

SyntaxRequest has a private prefix-retention switch supplied by the same publication boundary. Only source suffix changes may preserve accepted spans, and only while input remains eligible. Every new request still cancels the old subscription and advances its generation. Completion replaces spans atomically. Other input changes clear spans. The loader selects SelectableText from highlighting configuration, independent of asynchronous results.

No public interface or dependency changes. Built-in fenced-code following is independent of document navigation.
