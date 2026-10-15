pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QMarkdown 1.10
import QMarkdownViewer.Tools

ApplicationWindow {
    id: window
    width: 1000
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "QMarkdown playground"
    Component {
        id: hostTaskIndicator
        TaskIndicator { hostPalette: window.palette }
    }
    property string lastDestination: "No link activated"
    property int sampleIndex: 0
    property int previewWidth: 0
    readonly property string sample: samples.sources[sampleIndex]
    function loadSample() {
        editor.text = samples.sources[sampleIndex]
        editor.cursorPosition = 0
        sourceScroll.contentY = 0
        previewScroll.contentY = 0
    }
    function clearSource() { editor.text = ""; sourceScroll.contentY = 0; previewScroll.contentY = 0 }
    function switchStyle() { stylePanel.applyPreset(true) }
    function resetStyle() { stylePanel.resetStyle() }
    onSampleIndexChanged: loadSample()
    function openFind() { preview.searchActive = true; findQuery.forceActiveFocus(); findQuery.selectAll() }
    Shortcut { sequence: "Ctrl+F"; onActivated: window.openFind() }
    Shortcut { sequence: "Escape"; enabled: preview.searchActive; onActivated: { preview.searchActive = false; preview.forceActiveFocus() } }
    Samples { id: samples }
    Component {
        id: nativePreview
        Rectangle {
            id: customPreview
            objectName: "nativeFencePreview"
            required property string codeText
            required property string infoString
            required property string language
            required property var markdownStyle
            color: markdownStyle.codeBlock.backgroundColor
            implicitHeight: previewText.implicitHeight + 16
            Text {
                id: previewText
                x: 8
                y: 8
                width: Math.max(0, customPreview.width - 16)
                text: "Native preview: " + customPreview.codeText
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                color: customPreview.markdownStyle.codeBlockColor
                font: customPreview.markdownStyle.codeBlockFont
            }
        }
    }

    Clipboard { id: clipboard }
    MarkdownStyle {
        id: hostStyle
        objectName: "hostStyle"
        linkColor: window.palette.link
        bodyColor: window.palette.text
        h1Color: window.palette.text
        h2Color: window.palette.text
        h3Color: window.palette.text
        h4Color: window.palette.text
        h5Color: window.palette.text
        h6Color: window.palette.text
        codeBlockColor: window.palette.text
        thematicBreakColor: window.palette.text
        quoteRuleColor: window.palette.base.hslLightness < 0.5 ? "#a0a0a0" : "#707070"
    }
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 12
        spacing: 8
        RowLayout {
            Label { text: "Sample" }
            ComboBox {
                objectName: "sampleSelector"
                model: samples.names
                currentIndex: window.sampleIndex
                onActivated: window.sampleIndex = currentIndex
            }
            Button { text: "Reload sample"; objectName: "reloadButton"; onClicked: window.loadSample() }
            Button { text: "Clear"; objectName: "clearButton"; onClicked: window.clearSource() }
            Item { Layout.fillWidth: true }
            Label { text: "Preview width" }
            ComboBox {
                objectName: "widthSelector"
                model: ["Fit", "240", "480", "720"]
                onActivated: window.previewWidth = currentIndex === 0 ? 0 : Number(currentText)
            }
        }
        Flow {
            Layout.fillWidth: true
            spacing: 8
            Button { text: "Find"; onClicked: window.openFind() }
            CheckBox { id: outlineToggle; text: "Heading outline"; objectName: "outlineToggle" }
            CheckBox { id: rendererToggle; text: "Custom fences"; objectName: "rendererToggle" }
            CheckBox { id: selectionToggle; text: "Document selection"; checked: true; objectName: "selectionToggle" }
            CheckBox { id: styleToggle; text: "Style controls"; objectName: "styleToggle" }
            CheckBox { id: tablesToggle; text: "GFM tables"; objectName: "tablesToggle" }
            CheckBox { id: tagFilterToggle; text: "GFM tag filter"; objectName: "tagFilterToggle" }
            CheckBox { id: gfmToggle; text: "All GFM"; objectName: "gfmToggle" }
            CheckBox { id: autolinkToggle; text: "GFM extended autolinks"; objectName: "autolinkToggle" }
            CheckBox { id: strikeToggle; text: "GFM strikethrough"; objectName: "strikeToggle" }
            CheckBox { id: hostTaskToggle; text: "Host-styled task indicators"; checked: true; objectName: "hostTaskToggle" }
            CheckBox { id: taskToggle; text: "GFM task lists"; objectName: "taskToggle" }
        }
        RowLayout {
            visible: preview.searchActive
            Layout.fillWidth: true
            TextField {
                id: findQuery
                objectName: "findQuery"
                Layout.fillWidth: true
                placeholderText: "Find in document"
                onTextChanged: preview.searchQuery = text
                Keys.onReturnPressed: function(event) {
                    if (event.modifiers & Qt.ShiftModifier) preview.searchPrevious()
                    else preview.searchNext()
                }
                Keys.onEnterPressed: function(event) {
                    if (event.modifiers & Qt.ShiftModifier) preview.searchPrevious()
                    else preview.searchNext()
                }
            }
            CheckBox { text: "Match case"; onToggled: preview.searchCaseSensitive = checked }
            Label { objectName: "findCount"; text: (preview.currentSearchResult + 1) + " / " + preview.searchResultCount }
            Button { text: "Previous"; enabled: preview.searchResultCount > 0; onClicked: preview.searchPrevious() }
            Button { text: "Next"; enabled: preview.searchResultCount > 0; onClicked: preview.searchNext() }
            Button { text: "Close Find"; onClicked: { preview.searchActive = false; preview.forceActiveFocus() } }
        }
        ScrollView {
            visible: outlineToggle.checked
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(outlineItems.implicitHeight, 150)
            contentWidth: availableWidth
            clip: true
            Column {
                id: outlineItems
                width: parent.width
                Repeater {
                    model: preview.headings
                    delegate: Button {
                        required property var modelData
                        required property int index
                        width: outlineItems.width
                        text: "  ".repeat(modelData.level - 1) + modelData.title
                        flat: true
                        onClicked: preview.navigateToHeading(index)
                    }
                }
            }
        }
        ScrollView {
            id: styleScroll
            visible: styleToggle.checked
            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(stylePanel.implicitHeight, 220)
            clip: true
            contentWidth: availableWidth
            StylePanel {
                id: stylePanel
                objectName: "stylePanel"
                width: styleScroll.availableWidth
                targetStyle: hostStyle
                defaultLinkColor: window.palette.link
                defaultTextColor: window.palette.text
                previewBackgroundColor: window.palette.base
            }
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "Supports paragraphs, H1–H6, inline formatting, escapes/entities, code blocks, Setext headings, thematic breaks, lists and quotes. Links report destinations to the host; authorized PNG/JPEG images use native rows and HTML stays literal; all 652 pinned CommonMark semantic checks pass. Opt-in 1.1 presets retain host ownership and documented native presentation limits."
        }
        Label {
            objectName: "lastDestination"
            Layout.fillWidth: true
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            text: window.lastDestination
        }
        SplitView {
            id: split
            objectName: "paneDivider"
            Layout.fillWidth: true
            Layout.fillHeight: true
            orientation: Qt.Horizontal
            Pane {
                SplitView.preferredWidth: split.width / 2
                SplitView.minimumWidth: 200
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    Label { text: "Markdown source" }
                    Flickable {
                        id: sourceScroll
                        objectName: "sourceScroll"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: width
                        contentHeight: editor.height
                        ScrollBar.vertical: ScrollBar {}
                        TextArea {
                            id: editor
                            objectName: "editor"
                            width: sourceScroll.width
                            height: Math.max(sourceScroll.height, implicitHeight)
                            Component.onCompleted: window.loadSample()
                            textFormat: TextEdit.PlainText
                            wrapMode: TextEdit.Wrap
                            font: Qt.font({ pixelSize: 16 })
                            selectByMouse: true
                            onCursorRectangleChanged: {
                                if (cursorRectangle.y < sourceScroll.contentY) sourceScroll.contentY = cursorRectangle.y
                                else if (cursorRectangle.y + cursorRectangle.height > sourceScroll.contentY + sourceScroll.height)
                                    sourceScroll.contentY = cursorRectangle.y + cursorRectangle.height - sourceScroll.height
                            }
                        }
                    }
                }
            }
            Pane {
                background: Rectangle { objectName: "previewBackground"; color: window.palette.base }
                SplitView.fillWidth: true
                SplitView.minimumWidth: 200
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    RowLayout {
                        Label { objectName: "previewLabel"; text: "Native preview"; color: window.palette.text }
                        Item { Layout.fillWidth: true }
                        Button { text: "Select All"; onClicked: preview.selectAll() }
                        Button {
                            id: selectionCopy
                            objectName: "selectionCopyButton"
                            property bool copied: false
                            text: copied ? "Copied" : "Copy selection"
                            enabled: preview.selectedText.length > 0
                            onClicked: preview.requestCopy()
                            Timer { id: selectionFeedback; interval: 1500; onTriggered: selectionCopy.copied = false }
                        }
                    }
                    Flickable {
                        id: previewScroll
                        objectName: "previewScroll"
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        contentWidth: width
                        contentHeight: preview.contentHeight
                        ScrollBar.vertical: ScrollBar {}
                        MarkdownView {
                            keyboardNavigationEnabled: true
                            selectionEnabled: selectionToggle.checked
                            selectionViewport: previewScroll
                            onNavigationRequested: function(target) {
                                const mapped = preview.mapToItem(previewScroll.contentItem, target.x, target.y)
                                previewScroll.contentY = Math.max(0, Math.min(
                                    Math.max(0, previewScroll.contentHeight - previewScroll.height), mapped.y))
                            }
                            onSelectionScrollRequested: function(deltaY) {
                                previewScroll.contentY = Math.max(0, Math.min(
                                    Math.max(0, previewScroll.contentHeight - previewScroll.height),
                                    previewScroll.contentY + deltaY))
                            }
                            fencedBlockRenderers: rendererToggle.checked ? { preview: nativePreview } : ({})
                            onCopyRequested: function(text) { clipboard.copyText(text); selectionCopy.copied = true; selectionFeedback.restart() }
                            resourcePolicy: MarkdownResourcePolicy { allowQrc: true }
                            id: preview
                            objectName: "preview"
                            // Keep blank viewport space inside the selection hit area.
                            height: Math.max(previewScroll.height, contentHeight)
                            width: window.previewWidth > 0 ? Math.min(previewScroll.width, window.previewWidth) : previewScroll.width
                            x: (previewScroll.width - width) / 2
                            markdown: editor.text
                            tablesEnabled: tablesToggle.checked
                            tagFilterEnabled: tagFilterToggle.checked
                            gfmEnabled: gfmToggle.checked
                            taskListsEnabled: taskToggle.checked
                            taskIndicator: hostTaskToggle.checked ? hostTaskIndicator : null
                            extendedAutolinksEnabled: autolinkToggle.checked
                            strikethroughEnabled: strikeToggle.checked
                            style: hostStyle
                            codeBlockActions: Item {
                                id: actions
                                required property string codeText
                                required property var requestCopy
                                property bool copied: false
                                implicitHeight: copyButton.implicitHeight + 8
                                Button {
                                    id: copyButton
                                    objectName: "copyButton"
                                    width: Math.min(implicitWidth, actions.width)
                                    text: actions.copied ? "Copied" : "Copy"
                                    Accessible.name: "Copy code block"
                                    onClicked: { actions.requestCopy(); actions.copied = true; feedback.restart() }
                                }
                                Timer { id: feedback; interval: 1500; onTriggered: actions.copied = false }
                            }
                            onCodeBlockCopyRequested: function(text) { clipboard.copyText(text) }
                            onLinkActivated: function(destination) {
                                if (destination.startsWith("#")) {
                                    const found = preview.navigateToFragment(destination)
                                    window.lastDestination = (found ? "Local heading: " : "Missing heading: ") + destination
                                } else window.lastDestination = "Last activated: " + destination
                            }
                        }
                    }
                }
            }
        }
    }
}
