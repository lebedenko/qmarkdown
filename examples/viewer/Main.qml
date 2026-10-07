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
    MarkdownStyle { id: hostStyle; objectName: "hostStyle" }
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
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: "Supports paragraphs, H1–H6, inline formatting, escapes/entities and fenced code. Links, images and HTML remain literal; full CommonMark rendering is unfinished."
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
                background: Rectangle { color: "white" }
                SplitView.fillWidth: true
                SplitView.minimumWidth: 200
                padding: 8
                ColumnLayout {
                    anchors.fill: parent
                    Label { text: "Native preview"; color: "#202020" }
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
                            id: preview
                            objectName: "preview"
                            width: window.previewWidth > 0 ? Math.min(previewScroll.width, window.previewWidth) : previewScroll.width
                            x: (previewScroll.width - width) / 2
                            markdown: editor.text
                            style: hostStyle
                        }
                    }
                }
            }
        }
    }
}
