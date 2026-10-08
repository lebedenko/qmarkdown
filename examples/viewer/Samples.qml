import QtQml
QtObject {
    readonly property var names: ["Overview", "H1–H6 headings", "Inline formatting", "Fenced code", "Unicode and long lines", "Unsupported syntax", "Core leaf blocks"]
    readonly property var sources: [
        "# QMarkdown playground\n\nEdit the source to see native Qt Quick rendering immediately. Try **strong**, *emphasis*, and `inline code`.\n\n## Wrapping and styling\nUse the width selector and style controls to explore wrapping. Each pane scrolls independently.\n\n``` cpp\n// Literal fenced code\nQString greeting = \"Hello, 日本語 😀\";\n```\n\n### Supported today\nParagraphs, H1–H6, inline formatting, escapes/entities, code blocks, Setext headings and thematic breaks.\n\nLinks [example](https://example.invalid), images ![example](image.png), and <b>HTML</b> remain literal.",
        "# H1 with `code`\n\nBody paragraph.\n\n## H2 with *emphasis*\n\n### H3 with **strong**\n\n#### H4\n\n##### H5\n\n###### H6",
        "# Inline formatting\n\n*Emphasis*, **strong**, ***both***, and `literal <b> &amp;` code.\n\nEscapes: \\*literal asterisks\\* and \\# hash.\n\nEntities: &amp; &lt; &gt; &quot; &#169; &#x1F600;\n\n## Heading with `inherited code size`",
        "# Fenced code\n\n``` cpp &amp;\n# Literal Markdown, <b>HTML</b> and &amp;\n  Spaces stay;\ttabs stay too.\n\nAReallyLongUnbrokenCodeLineThatWrapsInsideTheAvailablePreviewWidthWithoutHorizontalScrolling1234567890\n```\n\n~~~ text\n日本語 😀 é\n~~~\n\nEmpty fence:\n```\n```",
        "# Unicode and long lines\n\né — Ελληνικά — 日本語 — 😀 🚀\n\n" + "LongUnbrokenLine1234567890".repeat(24) + "\n\n" + "A paragraph that wraps at the selected preview width. ".repeat(40),
        "# Unsupported syntax\n\n[link](https://example.invalid)\n\n![image](image.png)\n\n<b>Raw HTML</b>\n\n> Quote\n\n- List item\n- Another item\n\n| Table | Cell |\n| --- | --- |\n| A | B |",
        "Setext *H1*\n===\n\nMultiline\n**H2** with `code`\n---\n\nA paragraph interrupted by a rule.\n* * *\n\n    # Literal indented code\n    <b> &amp; [link](https://example.invalid)\n\n      Residual indentation and trailing spaces  \n\n___\n\n~~~\nFenced code shares the same style.\n~~~"
    ]
}
