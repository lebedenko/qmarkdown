# Design

Approved on 2026-10-10 through the explicit instruction to implement the supplied plan. Scope includes repository rules and release tooling; creating a tag or publishing the first release is excluded.

Reuse the existing pinned container and CMake install rules. Build a dedicated Release tree with tests enabled for verification, examples disabled; only CMake-installed files become payload. A standard-library Python installer behind shell entry points records file SHA-256, mode and symlink targets in /usr/share/qmarkdown/installed.json. Validate all inputs and every destination before writes; reject symlink ancestors and non-owned conflicts. Upgrades replace only unchanged recorded files and remove obsolete files. Uninstall keeps modified entries in the record for subsequent diagnosis. Real operations require root and refresh ldconfig; staging never invokes it.

Archive includes one enclosing directory, payload/usr, JSON manifest and metadata, instructions and installer. Tar preserves links and permissions. CI builds packages on each Qt but release publication selects the 6.8 package, then consumes that package in both toolchains. Upload compares downloaded existing assets byte-for-byte before uploading missing assets. No tag/release creation.

References: [rulesets](https://docs.github.com/en/repositories/configuring-branches-and-merges-in-your-repository/managing-rulesets/available-rules-for-rulesets), [release events](https://docs.github.com/en/actions/reference/workflows-and-actions/events-that-trigger-workflows), [DESTDIR](https://cmake.org/cmake/help/latest/envvar/DESTDIR.html).
