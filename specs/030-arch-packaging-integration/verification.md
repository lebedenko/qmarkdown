# Verification

## Planned

Focused Arch recipe and uploader tests; shell syntax; workflow structure; documentation links; JSON and historical log hashes; whitespace (excluding unchanged hashed historical log whitespace). Both packaging checks and complete manual release workflow on final PR revision and exact main merge SHA. Independently validate downloaded package/checksum pairs and source provenance. Preserve protections and existing release/tag. Retain final evidence separately.

## Actual

Local checks on 2026-10-12: all 40 Python tests passed (focused Arch recipe 5/5 and uploader 9/9 first); Bash syntax passed; release YAML parsed and upload dependencies/manual-run exclusion passed; prose documentation links passed; all iteration 029 retained evidence and corrected helper hashes matched. Tracked-change whitespace check passed. Hashed historical logs are preserved byte-for-byte. Hosted gates remain pending. Historical iteration 029 results remain in that iteration and do not satisfy these hosted integration gates.

Initial PR revision `3ebad6c293a0e0f6992aaef1a5b6480c8f84a38d`: packaging run 38239864288 failed because Ubuntu CI provisioning omitted Git, required by the archive-selection regression test. The minimum shared CTest run passed 11/12 checks; the archive test could not launch Git. Added Git to disposable CI provisioning; this does not change product dependencies. All revision-specific gates must run fresh. Failure evidence is retained by the hosted workflow and will be referenced in final evidence.
