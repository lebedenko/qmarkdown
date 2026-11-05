# Fragment experiment evidence

Outcome: **blocked**, not a successful fragment-aware layout implementation.

- [Summary](summary.json): exact disabled comparison across 751 fixtures / 775
  blocks, minimal cluster blocker, rejection checks, commands and input/output hashes.
- [Build](build.json): pinned source, checked patch, tools, dependency revisions,
  GN configuration, output hashes and unchanged original control identities.
- [Support](support.json): gate status and next architecture decision.
- [Object trace](historical-objects.jsonl): historical rounding/containment failure.
- `control-disabled.jsonl.gz` and `patched-disabled.jsonl.gz`: complete comparable
  geometry records. `minimal-*.jsonl.gz` retain joined and reshaped diagnostics.
- `control-images/` and `images/`: byte-identical CPU before/after diagnostic-patch
  captures; separately shaped characters are explicitly counterfactuals.
- `build.log`, `regression.log`, `supervisor.log`, `repository-check.log`: execution logs.
- [Artifact hashes](artifacts.json): collected evidence identities.

No padded geometry, fractional transition scan or native gate passed. Prior
placeholder and Qt Vulkan failures remain in their original evidence directories.
Use the [instructions and review launcher](../../demonstrator/fragment-experiment/README.md)
to reproduce or inspect. Owner visual acceptance remains separate.
