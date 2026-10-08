# 009: Design

**Approval:** On 2026-10-08 the user explicitly requested implementation of the supplied Feature 009 plan, approving these requirements, design and tasks for package 0.6.0 / module 0.6 before implementation.

Keep distinct UTF-16 link spans (start, length, destination) separate from formatting ranges while adapting the single cmark tree. Disable link collection within image descriptions, retaining any enclosing link. Propagate spans through the recursive block model; links select the cached FormattedText path.
Partition formatting at the union of formatting/link boundaries. Resolve existing font flags, then apply link color/underline. Hit testing uses cached QTextLine character cells, layout-to-paint translation and grapheme cursor boundaries, including bidi direction; reject line separators and trailing blank regions. Return integer identity (-1 for miss) separately from destination.
QML TapHandler uses DragThreshold and primary button. Remember pressed identity, validate release identity, and invalidate on text/span changes. HoverHandler recalculates hits after layout changes. Bound recursive components forward to the owning root signal.
