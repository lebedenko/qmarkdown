# 006: Design

**Approval:** The user explicitly instructed implementation of the supplied Feature 006 plan on 2026-10-08. Approval covers R1–R5, the design below, tasks T1–T6, and delivery as package 0.4.0 / QML module 0.4.

Extend the splitter with explicit ordinary/paragraph, fenced-code and indented-code states. Existing paragraph buffering identifies paragraph continuation. Fenced extraction remains unchanged. For indented code, buffer deindented blank lines separately and commit them only on a following code line; discard at termination/EOF. Every code content line carries a semantic LF, including EOF without LF, matching fenced model conventions. Process a terminating nonblank line in the ordinary path during the same iteration.

After code-state handling, check pending paragraph Setext underlines before thematic breaks, then ordinary fence/ATX/paragraph input. Underlines require contiguous markers and only trailing ASCII spaces/tabs. Thematic breaks count identical markers while allowing intervening ASCII spaces/tabs. Four-column input cannot match either syntax and cannot interrupt a paragraph.

Append private ThematicBreak kind and renderKind 3; omit inline adaptation for rules/code. Native Rectangle width follows the Column; explicit height uses finite/nonnegative thickness normalization. Zero-height rules follow the existing Qt Column behavior: they do not participate in positioning or spacing. Code reuses PlainText delegate. Style setters follow existing equality/NaN notification conventions and restoreDefaults restores both rule properties.

Playground gets an appended leaf-block sample (preserving existing sample indices), separate color/thickness controls, palette bindings, alternate accents, persistent manual overrides and reset. Code styling label becomes Code blocks. Update package/module/import/private registration and relocated consumer metadata to 0.4.0/0.4. No new abstraction or dependency; host still owns appearance.
