# Streaming Markdown (1.18)

Streaming is opt-in. Existing synchronous replacement behavior and imports remain supported.

```qml
import QMarkdown 1.18
MarkdownView {
    id: response
    streamingEnabled: true
}
// For each received chunk:
// response.appendMarkdown(chunk)
// At completion, or to publish immediately:
// response.flushMarkdown()
```

Alternatively bind `markdown` to a host-owned growing string. Strict suffix growth uses the same batching path. Choose one owner: calling `appendMarkdown()` writes the view’s source, while an existing binding can later overwrite that value; a host using a binding should append to its own source instead. Empty appends and identical assignments do nothing.

`markdown` and its notification reflect accepted input immediately. Derived blocks, selection/copy text, Find results, headings and links reflect the last published snapshot. A single 16 ms timer starts with the first pending update; later chunks do not restart it. This is a batching interval, with no frame-rate guarantee. `flushMarkdown()` synchronously parses and updates models and cancels that timer; image loading, highlighting, math and Qt Quick rendering remain asynchronous. Turning streaming off flushes pending input.

Clearing, shortening or assigning any non-prefix replacement discards the pending publication and applies the latest source immediately with ordinary replacement semantics. Semantic options, base URL and resource-policy changes consume pending source synchronously under the new settings.

Every published snapshot uses a complete static parse of the accepted source. Unfinished emphasis, links, fences and tables behave exactly as static partial source does. Later definitions may change earlier blocks. Streaming does not freeze completed paragraphs or add special syntax recovery.

Unchanged recursive rows and child models survive streaming publications and image completions. Retained images and requests keep their original deadlines and failure outcomes while resource settings remain unchanged. Replacement or resource-setting changes cancel the previous resource generation. Selections survive only across unchanged semantic ranges; surviving focused links and Find occurrences are retained without requesting automatic navigation.

Hosts own scrolling and following the stream. The view does not scroll on append or flush. Full parsing still scales with complete document size at each publication; batching and retained delegates/resources reduce repeated work but do not provide incremental parsing or guaranteed throughput.

The playground's Start stream, Stop / flush and Reset stream controls use deterministic chunks of the selected sample. The source editor owns the growing source binding.
