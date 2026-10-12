# Verification

## Planned

Shell syntax; focused recipe/version/checksum/uploader tests; exact-commit Arch build with CTest, QML lint and symbol checks; metadata/payload/dependency/license/link/path inspection and namcap; fresh pacman installation, registration, ownership and integrity; four installed consumers with default system discovery; higher-pkgrel upgrade and removal. Retain image ID, source SHA, installed package versions and logs. No release publication.

## Actual — 2026-10-12

- Python test discovery: **40 tests passed**, including recipe inputs, actual Git archive selection despite working-tree edits, installed payload boundaries, checksum/artifact selection and existing uploader/installer regressions.
- New CTest packaging entries: **2/2 passed** after configuring the existing shared build. Python compilation, Bash syntax, workflow YAML parsing/upload dependency inspection and `git diff --check` passed.
- Exact-commit package build: **10/10 source CTest checks passed**, QML lint passed, and **140 bundled external definitions prefixed with private shared exports** passed. Source commit `bc12ea98e33d98250358916ff0ca25197620164b` is the pre-change HEAD; packaging helpers were supplied from this implementation. The new Python checks were exercised separately on the working tree.
- Produced `qmarkdown-1.1.0-1-x86_64.pkg.tar.zst` with distribution Qt **6.12.0**, Release/shared, system install paths and no installed RPATH. Its SHA-256 companion passes `sha256sum -c`. This local candidate does not alter the published v1.1.0 release.
- Corrected fresh-container verification **passed**: identity/dependencies, installed payload/license locations, valid library links, root and pacman ownership, no build paths/static archives/Qt binaries/unresolved ELF dependencies, and `pacman -Qkk` reported zero altered files.
- All four installed CMake consumers **built and ran**: linked and plugin-only, each with/without styling. No custom library, QML, plugin or CMake search paths were supplied; plugin-only binaries did not link the backing library.
- Temporary unprivileged makepkg revision **1.1.0-2** installed as an upgrade, retained package ownership, then removal cleared the database entry and every owned file. Only revision **1** was exported.
- namcap reported **no errors**. Retained warnings include a missing Maintainer tag, the backing library being uninstalled at the pre-install analysis stage, and current Arch's split compiler-runtime packages satisfying gcc-libs implicitly. Actual installed ELF resolution and ownership passed.

[Machine-readable evidence](run-evidence.json) records source/archive/package hashes, immutable container identity, helper hashes and log hashes. Retained [build log](arch-build.log), [verification log](arch-verification.log), [build package versions](arch-build-packages.txt), [verification package versions](arch-verification-packages.txt) and [package metadata](arch-package-info.txt) substantiate these results. The artifact/checksum remain at `/tmp/qmarkdown-arch-029-snapshot/`.

Initial attempts exposed and corrected a helper mount-path error and a live-mounted shell helper changing during provisioning. The runner now snapshots helpers. The frozen snapshot's first package verification rejected installed private QML metadata; a corrected checker and regression test resolved this, and its fresh-container rerun passed using the same built artifact. Build and corrected verification helper hashes are recorded separately. No full rebuild was needed for this checker-only correction. The checksum was generated and validated after that successful rerun.

No hosted release workflow, Ubuntu rebuild, remote upload or release publication was performed. Workflow integration was parsed/reviewed locally; hosted behavior remains to be exercised on a manual or future stable release run. No package installation/removal occurred on the development host. Rolling repository evidence describes this run, not future byte reproducibility.
