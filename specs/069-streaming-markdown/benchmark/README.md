# Release streaming measurements

This standalone harness links the production library and constructs a native QML view. It is excluded from installation and the shipping build. Run from the repository root:

```sh
cmake -S specs/069-streaming-markdown/benchmark -B build-ci/streaming-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-ci/streaming-release --target streaming-measurements --parallel 2
QT_QPA_PLATFORM=offscreen QML_IMPORT_PATH="$PWD/build-ci/streaming-release/qml" build-ci/streaming-release/streaming-measurements > streaming-results.json
```

The ASCII documents are exactly 1,024 / 10,240 / 102,400 UTF-16 units and bytes. Chunks are 1,024 units; replacement settles every chunk, streaming explicitly flushes and settles every four chunks and at completion. The harness asserts publication/reset counts and decoder calls. Each document references one real authorized local PNG; every publication waits for resource completion. Image decodes measure first admission plus restarts, and every final view retains one decoded image.

Delegate counts observe direct Qt Quick children of recursive block sequences, excluding Repeaters. Creation/destruction connections avoid pointer-address reuse errors. Counts exclude final view teardown. Reset counts refer to the root model; focused identity tests separately cover retained child models. They do not count all helper items, allocations, GPU objects or scene-graph nodes.

[Recorded results](results-qt-6.12.0.json): host Qt 6.12.0, GCC 16.2.1, Release, offscreen, system default typography. There was one timed pass per case, without timing warmups; compilation used no compiler-cache launcher. Concurrent container verification introduced contention. Exploratory passes produced identical deterministic counts and different timings. Timing values are observations, not thresholds or throughput guarantees; the experiment combines batching and retention and does not isolate their contributions. Every publication still parses the full source.
