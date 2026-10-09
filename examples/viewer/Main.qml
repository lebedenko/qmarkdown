pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QMarkdown

ApplicationWindow {
    id: window
    width: 1000
    height: 700
    minimumWidth: 800
    minimumHeight: 600
    visible: true
    title: "QMarkdown playground"
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
    Samples { id: samples }
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
        CheckBox { id: styleToggle; text: "Style controls"; objectName: "styleToggle" }
        StylePanel {
            id: stylePanel
            objectName: "stylePanel"
            visible: styleToggle.checked
            Layout.fillWidth: true
            targetStyle: hostStyle
            defaultLinkColor: window.palette.link
            defaultTextColor: window.palette.text
            previewBackgroundColor: window.palette.base
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "Supports paragraphs, H1–H6, inline formatting, escapes/entities, code blocks, Setext headings, thematic breaks, lists and quotes. Links report destinations to the host; authorized PNG/JPEG images use native rows and HTML stays literal; all 652 pinned CommonMark semantic checks pass. The 1.0 candidate retains documented native presentation limits."
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
                    Label { objectName: "previewLabel"; text: "Native preview"; color: window.palette.text }
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
                            resourcePolicy: MarkdownResourcePolicy { allowQrc: true }
                            id: preview
                            objectName: "preview"
                            width: window.previewWidth > 0 ? Math.min(previewScroll.width, window.previewWidth) : previewScroll.width
                            x: (previewScroll.width - width) / 2
                            markdown: editor.text
                            style: hostStyle
                            onLinkActivated: function(destination) { window.lastDestination = "Last activated: " + destination }
                        }
                    }
                }
            }
        }
    }
}
