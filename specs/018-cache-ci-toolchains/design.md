# Design

Approved on 2026-10-09 through the explicit request to implement the supplied Iteration 018 plan. Approval covers requirements, design and tasks in this directory.

A dedicated scripts/ci-toolchain context contains a Dockerfile and common/Qt provisioning scripts. Common installation precedes the Qt ARG layer so both versions share dependency layers. Image provenance lives under /opt/qmarkdown-provenance. SDK qmake version and CMake package files are checked during construction.

The runner hashes sorted context filenames and contents plus Qt/platform, exposes JSON metadata before host/Docker checks, inspects the matching local tag and builds on a miss. Refresh uses --no-cache and --pull. Builds use a private context snapshot matching the calculated fingerprint. Verification resolves the tag to an immutable image ID. Preparation and verification have separate bounded subprocess lifetimes and logs; Ctrl-C terminates the private preparation process group (including Buildx plugin children) or stops the invocation-specific verification container. Runtime only copies provenance and creates writable directories before switching UID/GID.

GitHub uses the same metadata/context/build arguments and loads the image before the runner. Build action records and metadata join failure evidence. API v2 scopes distinguish Qt and amd64; ignore-error applies only to cache export. Cold builds remain valid. Fresh project builds remain mandatory.
