pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private
import "private/Layout.js" as Layout

Rectangle {
    id: surface
    required property string codeText
    required property string infoString
    required property bool fenced
    required property var style
    required property ViewState coordinator
    required property var view
    required property bool selected
    required property int projectionOffset
    required property int projectionLength
    property int textAlignment: Qt.AlignLeft
    readonly property real padX: Math.min(Layout.metric(style.codeBlock.horizontalPadding), Math.max(0, (width - 1) / 2))
    readonly property real padY: Layout.metric(style.codeBlock.verticalPadding)
    readonly property string language: {
        const match = /^\s*(\S+)/.exec(infoString)
        return match ? match[1] : ""
    }
    readonly property var rendererEntry: {
        const map = view.fencedBlockRenderers
        return !selected && fenced && language.length > 0 && map
            && Object.prototype.hasOwnProperty.call(map, language) ? map[language] : undefined
    }
    property Item rendererItem: null
    property var rendererToken: null
    property bool ready: false

    function clearRenderer() {
        rendererToken = null
        if (rendererItem) {
            rendererItem.visible = false
            rendererItem.destroy()
            rendererItem = null
        }
    }
    function rebuildRenderer() {
        clearRenderer()
        if (!ready || selected || !fenced || rendererEntry === undefined) return
        const component = rendererEntry as Component
        if (!component || component.status !== Component.Ready) {
            console.warn("QMarkdown: fenced renderer '" + language + "': invalid or unready Component; using code")
            return
        }
        const token = {}
        rendererToken = token
        const created = component.createObject(contents, {
            codeText: codeText,
            infoString: infoString,
            language: language,
            markdownStyle: Qt.binding(function() { return surface.style }),
            width: Qt.binding(function() { return contents.width })
        })
        // Host construction can synchronously replace the map/document/selection.
        if (rendererToken !== token) {
            if (created) {
                const partialItem = created as Item
                if (partialItem) partialItem.visible = false
                created.destroy()
            }
            return
        }
        if (!coordinator.validFencedRenderer(created)) {
            if (created) {
                const partialItem = created as Item
                if (partialItem) partialItem.visible = false
                created.destroy()
            }
            console.warn("QMarkdown: fenced renderer '" + language + "': must create an Item with required codeText, infoString, language and markdownStyle properties; using code")
            return
        }
        rendererItem = created as Item
        rendererItem.height = Qt.binding(function() { return Layout.metric(surface.rendererItem ? surface.rendererItem.implicitHeight : 0) })
    }
    Connections {
        target: surface.coordinator
        function onPresentationReset() { surface.ready = false; surface.clearRenderer() }
    }
    onRendererEntryChanged: rebuildRenderer()
    onCodeTextChanged: rebuildRenderer()
    onInfoStringChanged: rebuildRenderer()
    Component.onCompleted: { ready = true; rebuildRenderer() }
    Component.onDestruction: clearRenderer()
    readonly property string displayText: codeText.endsWith("\n") ? codeText.slice(0, -1) : codeText
    height: actions.height + (rendererItem ? Layout.metric(rendererItem.implicitHeight) : contents.height) + 2 * padY
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
        sourceComponent: surface.rendererItem ? null : surface.selected ? selectedText : plainText
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
