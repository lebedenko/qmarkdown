# Design

Approved on 2026-10-09 through the user’s explicit instruction to implement Iteration 016. Approval covers all five semantic distinctions, private implementation, independent evidence, and verification; package/module remain 0.7.0/0.7.

Adapt cmark’s single document tree into value-owned InlineNode vectors before projecting existing text/ranges/link/image spans. Containers keep children including empty link/image labels. Adaptation omits cmark’s empty Text artifacts left by trailing-space trimming; these carry no source semantics. Normalization itself preserves standalone empty Text nodes and only merges adjacent text. The semantic tree accompanies the flattened renderer projection: modest storage preserves semantics without redesigning rendering. Source blocks retain trees; image presentation slices construct only presentation fields.

The probe serializes exact nodes. Normalize adjacent Text siblings only, without crossing containers or breaks. The independent HTML oracle retains generated containers/breaks/title metadata and literal tokens; URL encoding and image-description interiors remain explicitly bounded. Source-authored cases compare exact trees reviewed against Markdown and CommonMark rules. HTML cannot distinguish entity-decoded LF from a source soft break when serialization emits a literal LF; retain source-authored checks for such cases rather than flattening either kind.

Probe schema 2, report schema 3 and authored schema 2 reject stale evidence. Review each ledger entry after checking presentation equality and semantic expectations. GFM, streaming, selection/copy, highlighting, optimization and remaining evidence limits are deferred.
