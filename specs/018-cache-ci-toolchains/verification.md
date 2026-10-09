# Verification

Planned: runner tests, shell/Python syntax, workflow validation, both Qt tasks twice with image IDs and timings compared. Hosted restoration requires hosted execution. Actual results will be recorded below.

## Actual checks (2026-10-09)

- `python3 -m unittest discover -s tests -p test_ci_runner.py -v`: 13 tests passed. Covers image reuse/miss/refresh, recipe/Qt/platform invalidation, source identity stability, read-only metadata without Docker, provenance identity/timings, preparation failure/timeout/cancellation (including child processes retaining stdout), verification timeout/cancellation, source isolation and evidence preservation.
- `bash -n scripts/ci-container.sh scripts/ci-toolchain/common.sh scripts/ci-toolchain/qt.sh`: passed.
- Python compilation and `git diff --check`: passed.
- Workflow parsed with PyYAML; all action references are 40-character SHA pins, embedded shell scripts pass `bash -n`, load/push and v2 cache settings checked. Actionlint v1.7.7 passed.
- Docker action tag SHAs resolved directly from upstream Git refs (setup-buildx v3.11.1, build-push v6.18.0).
- GitHub cache design checked against [Docker's GitHub cache guidance](https://docs.docker.com/build/cache/backends/gha/): explicit API v2, separate Qt/amd64 scopes, mode=max and ignore-error on export only. No hosted workflow was run; cache restoration/export, action execution and hosted artifact upload remain unverified.

Cold/warm task results are recorded below. Timings are observations, not speed guarantees.

## Local Docker task results

| Qt | Run | Result | Preparation (s) | Verification (s) | Total (s) |
| --- | --- | --- | ---: | ---: | ---: |
| 6.8.0 | cold | passed | 1245.137 | 114.107 | 1359.316 |
| 6.8.0 | warm | passed | 0.011 | 112.277 | 112.302 |
| 6.11.3 | failed cold | preparation failed | 1316.781 | 0 | 1316.782 |
| 6.11.3 | cold retry | passed | 1269.341 | 118.778 | 1388.155 |
| 6.11.3 | warm | passed | 0.012 | 119.856 | 119.882 |

Both versions passed cold and warm checks: shared/static CTest (7/7 each), QML lint, symbol privacy, installed/relocated consumers, library-only packaging and validated Release benchmark smoke. Existing nonfatal QML lint/CMake dependency warnings remain in logs.

The first Qt 6.11.3 preparation failed because the nic.funet.fi mirror timed out on ICU. It returned exit 1, retained image-preparation.log/result/environment evidence, and produced no source snapshot or verification log. A retry using cached common layers succeeded; no provisioning changes were needed.

Image identity stayed unchanged across each successful cold/warm pair:

- Qt 6.8.0: `sha256:1ad4a38e850a65c522afc76e2bc9d2713a27b4328ca685486422b0aecec40ff6`.
- Qt 6.11.3: `sha256:f4725ca42c718e11d607116b5cb912d2a854d26b97045334897297b0196f7b18`.

Warm preparation logs contain only the reuse record; verification logs contain no apt/pip/Qt installation commands. Each pair has identical recipe fingerprints and byte-identical aqt logs, installer/Ubuntu package manifests, OS information and Qt-version provenance copied from the image. Tool evidence confirms UID/GID 1000 and read-only source mounts. Project snapshots, packaging directories and benchmark builds are fresh for every run. Docker reported the common dependency layer cached for the second Qt version and the retry.

[Run evidence](run-evidence.json) retains fingerprints, immutable IDs, relative artifact paths and precise timings for all five attempts. Full local logs/build reports remain in those ignored build-ci directories. Hosted cache restoration remains unverified.

## Hosted bootstrap follow-up (2026-10-10)

[GitHub run 37993963391](https://github.com/lebedenko/qmarkdown/actions/runs/37993963391), at commit `bf5c58cf14ebad3aed13568f938d41b4a01d1a41`, failed for both Qt versions before toolchain preparation or project tests. The original attempt and `gh run rerun 37993963391 --failed` both timed out fetching a Docker Hub authentication token while pulling `moby/buildkit:buildx-stable-1` in setup-buildx-action.

The workflow now bootstraps BuildKit from `mirror.gcr.io/moby/buildkit:buildx-stable-1` and configures `mirror.gcr.io` as the Docker Hub registry mirror using [Docker's documented BuildKit configuration](https://docs.docker.com/build/buildkit/configure/#registry-mirror). Action pins, Qt versions, the pinned Ubuntu base, and cache settings remain unchanged. Google's mirror caches public images; its availability and cache retention are not guaranteed.

Actual local checks:

- Workflow YAML parsed with PyYAML; inline BuildKit configuration parsed with `tomllib`. The pinned setup-buildx action declares both configuration inputs. Actionlint was unavailable for this follow-up.
- `python3 tests/test_ci_runner.py`: 13 tests passed.
- `docker buildx imagetools inspect mirror.gcr.io/moby/buildkit:buildx-stable-1`: passed; observed index digest `sha256:cec9f139f45e93c5c69c60f8b07cfad9f43f4ef6b6a6cd917527fea5ff2e3dea`.
- `docker buildx create --name qmarkdown-gh-mirror-37993963391 --driver docker-container --driver-opt image=mirror.gcr.io/moby/buildkit:buildx-stable-1 --buildkitd-config /tmp/qmarkdown-buildkit-mirror.toml --bootstrap`: passed. Inspection confirmed running BuildKit v0.33.1 and the registry mirror configuration.
- `docker buildx build --builder qmarkdown-gh-mirror-37993963391 --platform linux/amd64 --progress plain --output=type=cacheonly /tmp/qmarkdown-buildkit-mirror-smoke`: passed. The smoke Dockerfile used the unchanged toolchain Ubuntu digest `sha256:f610ab94648195aa356059f5b41d6085c9d4d903c072430cdd1af7bdb646106b` and checked `/etc/os-release`.
- `git diff --check`: passed.

Logs remain locally under `/tmp/qmarkdown-gh-failed-37993963391.log`, `/tmp/qmarkdown-gh-rerun-37993963391.log`, and `/tmp/qmarkdown-buildkit-mirror-smoke.log`. Hosted execution of the patched workflow and hosted cache restoration remain pending until publication of this change. The existing 1.0 candidate archive and its recorded verification snapshot are unchanged.
