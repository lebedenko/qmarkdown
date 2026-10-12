# Arch Linux release package

The v1.2.0 release provides `qmarkdown-1.2.0-1-x86_64.pkg.tar.zst` and a
SHA-256 companion after Ubuntu and Arch verification succeeds. The existing
v1.1.0 release assets are unchanged. Arch packages use the distribution's Qt 6
runtime; pacman enforces Qt lower bounds matching the package's build version.
Qt is not bundled. The viewer is not packaged.

Download the [package](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-1-x86_64.pkg.tar.zst) and [checksum](https://github.com/lebedenko/qmarkdown/releases/download/v1.2.0/qmarkdown-1.2.0-1-x86_64.pkg.tar.zst.sha256), and verify before installation:

```sh
sha256sum -c qmarkdown-1.2.0-1-x86_64.pkg.tar.zst.sha256
sudo pacman -U ./qmarkdown-1.2.0-1-x86_64.pkg.tar.zst
pacman -Qi qmarkdown
pacman -Qo /usr/lib/libQMarkdown.so
```

Pacman owns
the shared library, QML plugin/metadata, CMake configuration and licenses, and
tracks runtime dependencies. The system locations are `/usr/lib`,
`/usr/lib/qt6/qml/QMarkdown`, `/usr/lib/cmake/QMarkdown` and
`/usr/share/licenses/qmarkdown`. Arch's Qt discovers these paths normally;
no custom library or QML search path is required. Development consumers need
CMake and a C++17 compiler. Existing public CMake and QML versions are unchanged.

Install a later package using the same `pacman -U` command. Remove it with:

```sh
sudo pacman -R qmarkdown
```

See the [pacman manual](https://man.archlinux.org/man/pacman.8.en) for package
inspection and lifecycle commands. Do not overlay the Ubuntu custom installer
on pacman-owned files.

## Local building

From a source checkout, with Python 3, Git, Docker and access to its daemon:

```sh
python3 scripts/build-arch-package.py --source-commit HEAD --work-dir /tmp/qmarkdown-arch-build
```

Choose a new or empty output directory. The runner archives the selected commit,
not working-tree edits; packaging tools are snapshotted from the current checkout at run start. `--pkgrel`
defaults to `1` and accepts a positive integer. Containers are disposable x86_64
Arch environments with fully updated repositories. makepkg runs as an
unprivileged account and validates the source checksum. The PKGBUILD uses
[makepkg's package/check lifecycle](https://man.archlinux.org/man/makepkg.8.en)
and disables a separate debug package.

The output contains the verified package/checksum, source archive and checksum
identity, rendered PKGBUILD, source commit, immutable container identity,
installed package versions and verification logs. CTest, QML lint, parser symbol
privacy, namcap, actual pacman ownership/integrity, installed consumers and a
temporary higher-pkgrel upgrade/removal are release gates. The upgrade artifact
stays inside the verification container. No packages are installed on the host.
Manual release-workflow runs perform these checks without publication.

Current rolling repositories are intentional. Recorded environment evidence
does not promise identical bytes from a later rebuild. No AUR publishing,
package repository hosting or signing infrastructure is included.

The v1.2.0 publication and downloaded-asset results are recorded in [iteration 033](../specs/033-stable-v1.2-release/verification.md); publication and independent downloaded-asset acceptance passed.
