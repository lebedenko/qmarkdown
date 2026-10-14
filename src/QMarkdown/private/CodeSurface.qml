pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private
import "private/Layout.js" as Layout

Rectangle {
    id: surface
    required property string codeText
    required property var style
    required property ViewState coordinator
    required property var view
    required property bool selected
    required property int projectionOffset
    required property int projectionLength
    property int textAlignment: Qt.AlignLeft
    readonly property real padX: Math.min(Layout.metric(style.codeBlock.horizontalPadding), Math.max(0, (width - 1) / 2))
    readonly property real padY: Layout.metric(style.codeBlock.verticalPadding)
    readonly property string displayText: codeText.endsWith("\n") ? codeText.slice(0, -1) : codeText
    height: actions.height + contents.height + 2 * padY
    color: style.codeBlock.backgroundColor
    border.color: style.codeBlock.borderColor
    border.width: Layout.metric(style.codeBlock.borderWidth)
    radius: Layout.metric(style.codeBlock.cornerRadius)
    objectName: "codeSurface"

    CodeActionHost {
        id: actions
        x: surface.padX
        y: surface.padY
        width: Math.max(1, surface.width - 2 * surface.padX)
        codeText: surface.codeText
        view: surface.view
        coordinator: surface.coordinator
    }
    Loader {
        id: contents
        x: surface.padX
        y: surface.padY + actions.height
        width: Math.max(1, surface.width - 2 * surface.padX)
        sourceComponent: surface.selected ? selectedText : plainText
    }
    Component {
        id: plainText
        Text {
            text: surface.displayText
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            horizontalAlignment: surface.textAlignment
            elide: Text.ElideNone
            font: surface.style.codeBlockFont
            color: surface.style.codeBlockColor
            height: text.length === 0 ? metrics.height : implicitHeight
            FontMetrics { id: metrics; font: surface.style.codeBlockFont }
        }
    }
    Component {
        id: selectedText
        FormattedText {
            id: leaf
            property int projectionOffset: surface.projectionOffset
            property int projectionLength: surface.projectionLength
            text: surface.displayText
            layoutWidth: contents.width
            horizontalAlignment: surface.textAlignment
            font: surface.style.codeBlockFont
            color: surface.style.codeBlockColor
            selectionStart: Math.max(0, surface.coordinator.selectionStart - projectionOffset)
            selectionEnd: Math.max(0, surface.coordinator.selectionEnd - projectionOffset)
            selectionColor: surface.style.selectionBackgroundColor
            implicitHeight: logicalHeight
            Component.onCompleted: surface.coordinator.registerSelectionItem(leaf, ViewState.TextSelection)
            Component.onDestruction: surface.coordinator.unregisterSelectionItem(leaf)
        }
    }
}
