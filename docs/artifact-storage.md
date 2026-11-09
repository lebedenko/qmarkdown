# Repository artifact storage

Git stores source, tests, fixtures needed by those tests, specifications, and small
human-readable results. It is not a store for build products or raw experiment runs.

- Keep build trees, SDK copies, compiled libraries/executables, caches and generated
  captures in ignored `build*/` or `artifacts/` directories, or outside the checkout.
- Do not commit archives, split archives, generated screenshots/videos, raw logs,
  JSONL traces or bulk analysis dumps. Splitting a file does not make it suitable
  for Git. Small JSON summaries and checksums may accompany a written conclusion.
- Keep repository files at or below 1 MiB. Existing source assets such as vendored
  fonts and example images are retained; generated experiment captures are not
  source assets. A necessary larger source fixture requires an explicit policy
  change and review, not a silent bypass.
- Use an explicitly chosen external artifact destination when raw evidence must
  be shared. No external destination or upload is implied by a request to preserve
  evidence. Record a URL, checksum, purpose and expiry in the concise report.
- Keep only the active build/diagnostic data needed for current work. At delivery,
  remove obsolete copies and report retained local disk usage. Do not duplicate
  SDKs and run directories solely to record another verification result.

`python3 scripts/check-storage.py` checks tracked and non-ignored new files.
`task check` and staged-snapshot pre-commit checks run the same policy, including
files force-added despite `.gitignore`. The rule applies to compressed and
uncompressed generated data alike.

## Owner-authorized cleanup, 2026-11-09

The owner explicitly requested removal of generated binaries and archives from
**all history**, superseding earlier instructions to preserve raw evidence archives
in this repository. The cleanup removes 4,699 historical artifact paths from all
local branches, remote-tracking refs and tags. Retained path/blob/mode lists at all
31 ref tips were verified against their pre-rewrite counterparts. Commit IDs have
changed; Git's local `.git/filter-repo/commit-map` records the translation.

Historical prose, result summaries and checksums remain records of what was run;
they are not claims that removed raw evidence is still available. Links to removed
artifacts are rendered as historical paths. Archive-based historical revalidation
and tests needing deleted raw captures require those inputs to be regenerated;
they must not be reported as passing merely because a summary remains.

Shipping source, test code, vendored dependencies/fonts and example assets were
preserved. The scale-contract generator's small, hash-pinned source baseline is
now tracked directly, so source generation no longer needs an ignored build tree
or a deleted evidence archive. Compiled experiment dependencies must be rebuilt.

Rewritten published history requires coordinated remote updates and fresh clones
or deliberate local migration; do not merge old history back into the cleaned
repository. Cleanup does not modify release download assets or external caches.

Local verification: all remaining reachable history blobs are below 1 MiB;
`git fsck --full --no-reflogs` passes. The complete Python/Light suite passes
(91 tests), including storage-policy and staged-snapshot coverage. Eight focused
scale-contract tests and the independent C++ mapping check pass after moving the
source baseline. Shipping builds were not rerun: shipping source is byte-identical,
and rebuilding deleted SDK/cache trees is not required to verify data removal.

The local checkout was reduced from approximately 83 GiB to 44 MiB, including
about 5.6 MiB of Git metadata. No old-history bundle or replacement archive was
created. Generated build directories were removed, including foreign-owned SDK
files removed by the owner after local sudo required a password.
