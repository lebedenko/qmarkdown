# Code-block actions (1.2)

Import `QMarkdown 1.2` and assign `MarkdownView.codeBlockActions` to a Component whose root is an Item. It must declare `required property string codeText` and `required property var requestCopy`. Call `requestCopy()` to emit `codeBlockCopyRequested(string text)` on that view. The default is null.

```qml
import QtQuick
import QtQuick.Controls
import QMarkdown 1.2

MarkdownView {
    width: 480
    markdown: "```\n  literal code\n```"
    codeBlockActions: Item {
        id: actions
        required property string codeText
        required property var requestCopy
        implicitHeight: button.implicitHeight + 8
        Button {
            id: button
            width: Math.min(implicitWidth, actions.width)
            text: "Copy"
            Accessible.name: "Copy code block"
            onClicked: actions.requestCopy()
        }
    }
    onCodeBlockCopyRequested: function(text) { hostClipboard.copyText(text) }
}
```

`hostClipboard` is supplied by the application. The library never writes the clipboard and does not depend on Qt Quick Controls. Controls in this example belong to the host. The playground supplies a native helper that writes plain text and a Button with short “Copied” feedback.

`codeText` and the emitted signal contain complete parser-produced code text, including literal whitespace and trailing newlines. Fences and info strings are excluded. No additional normalization is performed. Displayed code still omits its final newline for layout; copying preserves it. Every fenced or indented block receives its own header, including empty blocks and blocks nested in lists/quotes.

The header is above the code, inside the existing surface padding. The library controls its width and height: width equals the available content width (at least one pixel at positive view widths), height reserves finite nonnegative `implicitHeight` (negative, NaN and infinite values reserve zero). Hosts provide spacing within that height, styling, accessibility labels, focus behavior, clipboard handling and feedback. Null headers preserve existing dimensions. A control wider/taller than its allotted geometry can overflow; hosts should size children appropriately.

Component or block replacement destroys old instances and invalidates their callbacks. Zero view width removes blocks; restoring positive width creates fresh headers. Callbacks retained after removal or view destruction perform no request. Delegate creation errors produce diagnostics and leave ordinary code rendering usable. Components must retain a valid creation context for their instances, as required by Qt.

Package 1.2.0 accepts compatible 1.0/1.1/1.2 CMake requests and EXACT 1.2.0. EXACT 1.0.0/1.1.0 requests require their historical packages. Existing QML 1.0/1.1 consumers remain supported. See [release notes](release-notes-1.2.0.md) and [integration verification](../specs/032-v1.2-integration/verification.md). [Iteration 033](../specs/033-stable-v1.2-release/verification.md) tracks pending stable publication. Document-wide selection, inline-code actions, language labels, highlighting and generic renderer extensions remain deferred.
