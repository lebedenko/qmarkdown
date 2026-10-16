pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
import QMarkdown.Private
import "private/Layout.js" as Layout

Rectangle {
    id: surface
    required property string codeText
    required property string infoString
    required property bool fenced
    required property MarkdownStyle style
    required property ViewState coordinator
    required property Component actions
    required property var rendererRegistry
    required property string sourceIdentity
    required property bool viewportAvailable
    required property bool searchActive
    required property color searchHighlightColor
    required property color currentSearchHighlightColor
    required property string language
    signal copyRequested(string text)
    required property bool selected
    required property int projectionOffset
    required property int projectionLength
    property int textAlignment: Qt.AlignLeft
    readonly property real padX: Math.min(Layout.metric(style.codeBlock.horizontalPadding), Math.max(0, (width - 1) / 2))
    readonly property real padY: Layout.metric(style.codeBlock.verticalPadding)
    readonly property var rendererEntry: {
        const map = surface.rendererRegistry
        return !selected && !surface.searchActive && fenced && language.length > 0 && map
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
        if (!ready || selected || surface.searchActive || !fenced || rendererEntry === undefined) return
        const component = rendererEntry as Component
        if (!component || component.status !== Component.Ready) {
            console.warn("QMarkdown: fenced renderer '" + language + "': invalid or unready Component; using code")
            return
        }
        const token = {}
        rendererToken = token
        const created = component.createObject(rendererHost, {
            codeText: codeText,
            infoString: infoString,
            language: language,
            markdownStyle: Qt.binding(function() { return surface.style }),
            width: Qt.binding(function() { return rendererHost.width })
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
    onCodeTextChanged: {
        firstSourcePosition = 0; firstSourceOffset = 0; firstSourceOffsetX = 0
        viewport.contentX = 0; viewport.contentY = 0
        rebuildRenderer()
    }
    onInfoStringChanged: rebuildRenderer()
    Component.onCompleted: { ready = true; rebuildRenderer() }
    Component.onDestruction: clearRenderer()
    readonly property string displayText: codeText.endsWith("\n") ? codeText.slice(0, -1) : codeText
    readonly property bool wrapEnabled: coordinator.codeBlockWrapEnabled
    readonly property bool numbersVisible: coordinator.codeBlockLineNumbersVisible && !rendererItem
    readonly property real gutterWidth: {
        if (!numbersVisible) return 0
        const count = Math.max(1, displayText.split("\n").length)
        let widest = 0
        for (let number = 1; number <= count; ++number)
            widest = Math.max(widest, metrics.advanceWidth(String(number)))
        return Math.ceil(widest) + 16
    }
    readonly property var sourceItem: contents.item
    readonly property real naturalHeight: surface.sourceItem ? surface.sourceItem.sourceHeight : 0
    readonly property real naturalWidth: surface.sourceItem ? surface.sourceItem.sourceWidth : 0
    readonly property real lineHeight: Math.max(1, surface.sourceItem ? surface.sourceItem.sourceLineHeight : metrics.height)
    readonly property real minimumLines: Math.max(1, coordinator.codeBlockMinimumLines)
    readonly property real maximumLines: Math.max(minimumLines, coordinator.codeBlockMaximumLines)
    readonly property real heightLimit: {
        const viewport = coordinator.codeBlockViewportHeight
        const ratio = coordinator.codeBlockHeightRatio
        const desired = Number.isFinite(viewport) && viewport > 0
            ? viewport * (Number.isFinite(ratio) && ratio > 0 ? ratio : 0.5) : maximumLines * lineHeight
        return Math.max(minimumLines * lineHeight, Math.min(maximumLines * lineHeight, desired))
    }
    readonly property bool verticalOverflow: coordinator.codeBlockHeightLimitEnabled && naturalHeight > heightLimit
    readonly property bool horizontalOverflow: !wrapEnabled && naturalWidth > viewport.width
    property int firstSourcePosition: 0
    property real firstSourceOffset: 0
    property real firstSourceOffsetX: 0
    property bool restoring: false
    property bool restoreQueued: false
    function rememberPosition() {
        if (restoring || restoreQueued || !surface.sourceItem) return
        if (viewport.contentX === 0 && viewport.contentY === 0) {
            firstSourcePosition = 0; firstSourceOffset = 0; firstSourceOffsetX = 0
            return
        }
        firstSourcePosition = surface.sourceItem.sourcePosition(viewport.contentX, viewport.contentY)
        const rect = surface.sourceItem.sourceRectangle(firstSourcePosition)
        firstSourceOffset = viewport.contentY - rect.y
        firstSourceOffsetX = viewport.contentX - rect.x
    }
    function restorePosition() {
        if (restoreQueued) return
        restoreQueued = true
        Qt.callLater(function() {
            if (!surface.ready) return
            surface.restoreQueued = false
            surface.restoring = true
            if (surface.firstSourcePosition === 0 && surface.firstSourceOffset === 0 && surface.firstSourceOffsetX === 0) {
                viewport.contentX = 0; viewport.contentY = 0
                surface.restoring = false
                return
            }
            if (surface.sourceItem) viewport.contentY = Math.max(0, Math.min(viewport.contentHeight - viewport.height,
                surface.sourceItem.sourceRectangle(surface.firstSourcePosition).y + surface.firstSourceOffset))
            viewport.contentX = surface.wrapEnabled || !surface.sourceItem ? 0 : Math.max(0, Math.min(surface.sourceItem.sourceRectangle(surface.firstSourcePosition).x + surface.firstSourceOffsetX, viewport.contentWidth - viewport.width))
            surface.restoring = false
        })
    }
    onNaturalHeightChanged: restorePosition()
    onWidthChanged: restorePosition()
    onWrapEnabledChanged: restorePosition()
    height: actionHost.height + (rendererItem ? Layout.metric(rendererItem.implicitHeight) : viewport.height + (horizontalOverflow ? 10 : 0)) + 2 * padY
    FontMetrics { id: metrics; font: surface.style.codeBlockFont }
    color: style.codeBlock.backgroundColor
    border.color: style.codeBlock.borderColor
    border.width: Layout.metric(style.codeBlock.borderWidth)
    radius: Layout.metric(style.codeBlock.cornerRadius)
    objectName: "codeSurface"

    CodeActionHost {
        id: actionHost
        x: surface.padX
        y: surface.padY
        width: Math.max(1, surface.width - 2 * surface.padX)
        language: surface.language
        infoString: surface.infoString
        fenced: surface.fenced
        codeText: surface.codeText
        actions: surface.actions
        sourceIdentity: surface.sourceIdentity
        viewportAvailable: surface.viewportAvailable
        onCopyRequested: function(text) { surface.copyRequested(text) }
        coordinator: surface.coordinator
    }
    SyntaxRequest {
        id: syntaxRequest
        source: surface.codeText
        language: surface.language
        enabled: surface.fenced && surface.coordinator.syntaxHighlightingEnabled && !surface.rendererItem
        theme: surface.coordinator.syntaxTheme
    }
    Item {
        id: rendererHost
        x: surface.padX; y: surface.padY + actionHost.height
        width: Math.max(1, surface.width - 2 * surface.padX)
        height: surface.rendererItem ? Layout.metric(surface.rendererItem.implicitHeight) : 0
    }
    Flickable {
        id: viewport
        objectName: "codeViewport"
        property bool codeSourceViewport: true
        Component.onCompleted: surface.coordinator.registerSourceViewport(viewport)
        x: surface.padX + surface.gutterWidth
        y: surface.padY + actionHost.height
        width: Math.max(1, surface.width - 2 * surface.padX - surface.gutterWidth - (surface.verticalOverflow ? 10 : 0))
        height: surface.rendererItem ? 0 : Math.max(0, surface.coordinator.codeBlockHeightLimitEnabled ? Math.min(surface.naturalHeight, surface.heightLimit) : surface.naturalHeight)
        contentWidth: surface.wrapEnabled ? width : Math.max(width, surface.naturalWidth)
        contentHeight: surface.naturalHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: !surface.selected
        DragHandler {
            enabled: surface.selected
            acceptedDevices: PointerDevice.TouchScreen
            target: null
            property point previous: Qt.point(0, 0)
            onActiveChanged: previous = Qt.point(0, 0)
            onActiveTranslationChanged: {
                if (!active) return
                const dx = previous.x - activeTranslation.x, dy = previous.y - activeTranslation.y
                previous = activeTranslation
                const oldY = viewport.contentY
                viewport.contentX = Math.max(0, Math.min(viewport.contentX + dx, viewport.contentWidth - viewport.width))
                viewport.contentY = Math.max(0, Math.min(oldY + dy, viewport.contentHeight - viewport.height))
                const remaining = dy - (viewport.contentY - oldY)
                if (remaining !== 0) surface.coordinator.selectionScrollRequested(remaining)
            }
        }
        onContentYChanged: surface.rememberPosition()
        onContentXChanged: surface.rememberPosition()
        function revealSource(rect: rect) {
            if (rect.y < contentY) contentY = rect.y
            else if (rect.y + rect.height > contentY + height) contentY = rect.y + rect.height - height
            if (rect.x < contentX) contentX = rect.x
            else if (rect.x + rect.width > contentX + width) contentX = rect.x + rect.width - width
            contentY = Math.max(0, Math.min(contentY, contentHeight - height))
            contentX = Math.max(0, Math.min(contentX, contentWidth - width))
        }
        Loader {
            id: contents
            width: surface.wrapEnabled ? viewport.width : Math.max(viewport.width, surface.naturalWidth)
            sourceComponent: surface.rendererItem ? null : surface.selected || surface.searchActive || surface.numbersVisible || syntaxRequest.ranges.length > 0 ? selectedText : plainText
        }
    }
    Item {
        id: gutter
        LayoutMirroring.enabled: false
        x: surface.padX
        y: viewport.y
        width: surface.gutterWidth
        height: viewport.height
        visible: surface.numbersVisible
        clip: true
        Repeater {
            model: surface.numbersVisible && surface.sourceItem ? surface.sourceItem.visibleSourceLines : []
            delegate: Text {
                required property var modelData
                text: modelData.number
                font: surface.style.codeBlockFont
                color: surface.style.codeBlock.lineNumberColor
                x: 8
                width: Math.max(0, gutter.width - 16)
                y: modelData.baseline - baselineOffset - viewport.contentY
                horizontalAlignment: Text.AlignRight
                Accessible.ignored: true
            }
        }
        SelectionRegistration { coordinator: surface.coordinator; item: gutter; kind: ViewState.ActionSelection }
    }
    WheelHandler {
        target: null
        blocking: false
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        onWheel: function(event) {
            const dx = event.pixelDelta.x !== 0 ? -event.pixelDelta.x : -event.angleDelta.x / 120 * surface.lineHeight * 3
            const dy = event.pixelDelta.y !== 0 ? -event.pixelDelta.y : -event.angleDelta.y / 120 * surface.lineHeight * 3
            const oldX = viewport.contentX, oldY = viewport.contentY
            if (event.modifiers & Qt.ShiftModifier) viewport.contentX = Math.max(0, Math.min(oldX + dy + dx, viewport.contentWidth - viewport.width))
            else {
                viewport.contentX = Math.max(0, Math.min(oldX + dx, viewport.contentWidth - viewport.width))
                viewport.contentY = Math.max(0, Math.min(oldY + dy, viewport.contentHeight - viewport.height))
            }
            event.accepted = oldX !== viewport.contentX || oldY !== viewport.contentY
        }
    }
    component SourceScrollBar: Item {
        id: bar
        required property int orientation
        readonly property bool vertical: orientation === Qt.Vertical
        readonly property real extent: vertical ? viewport.contentHeight : viewport.contentWidth
        readonly property real span: vertical ? viewport.height : viewport.width
        readonly property real position: extent > 0 ? (vertical ? viewport.contentY : viewport.contentX) / extent : 0
        readonly property real size: extent > 0 ? Math.min(1, span / extent) : 1
        readonly property bool active: viewport.moving || barHover.hovered
        property Item hostItem: null
        property var callbackToken: null
        function requestPosition(value: real) {
            if (!Number.isFinite(value)) return
            const offset = Math.max(0, Math.min(1 - size, value)) * extent
            if (vertical) viewport.contentY = offset
            else viewport.contentX = offset
        }
        function rebuild() {
            if (callbackToken) callbackToken.active = false
            const validity = { active: true }
            callbackToken = validity
            if (hostItem) { hostItem.visible = false; hostItem.destroy(); hostItem = null }
            const component = surface.coordinator.codeBlockScrollBar
            if (!visible || !component) return
            const created = surface.coordinator.createHostComponent(component, bar, {
                orientation: orientation, position: position, size: size, active: active,
                requestPosition: function(value) { if (validity.active) bar.requestPosition(value) }
            })
            if (!validity.active) { if (created) created.destroy(); return }
            const item = created as Item
            if (!surface.coordinator.validCodeScrollBar(item)) {
                validity.active = false
                if (created) created.destroy()
                return
            }
            hostItem = item
            item.width = Qt.binding(function() { return bar.width })
            item.height = Qt.binding(function() { return bar.height })
        }
        HoverHandler { id: barHover }
        Binding { target: bar.hostItem; property: "position"; value: bar.position; when: bar.hostItem !== null }
        Binding { target: bar.hostItem; property: "size"; value: bar.size; when: bar.hostItem !== null }
        Binding { target: bar.hostItem; property: "active"; value: bar.active; when: bar.hostItem !== null }
        onVisibleChanged: rebuild()
        Component.onCompleted: rebuild()
        Component.onDestruction: { if (callbackToken) callbackToken.active = false }
        Connections { target: surface.coordinator; function onCodeBlockSettingsChanged() { bar.rebuild() } }
        Rectangle {
            visible: !bar.hostItem
            color: surface.style.codeBlock.lineNumberColor
            opacity: bar.active ? 0.8 : 0.4
            radius: 3
            x: bar.vertical ? 2 : bar.position * bar.width
            y: bar.vertical ? bar.position * bar.height : 2
            width: bar.vertical ? 6 : Math.max(6, bar.size * bar.width)
            height: bar.vertical ? Math.max(6, bar.size * bar.height) : 6
        }
        MouseArea {
            id: pointer
            anchors.fill: parent
            enabled: !bar.hostItem
            hoverEnabled: true
            function moveTo(mouse) { bar.requestPosition((bar.vertical ? mouse.y / height : mouse.x / width) - bar.size / 2) }
            onPressed: function(mouse) { moveTo(mouse) }
            onPositionChanged: function(mouse) { if (pressed) moveTo(mouse) }
        }
        SelectionRegistration { coordinator: surface.coordinator; item: bar; kind: ViewState.ActionSelection }
    }
    SourceScrollBar {
        orientation: Qt.Vertical
        x: viewport.x + viewport.width
        y: viewport.y
        width: 10; height: viewport.height
        visible: surface.verticalOverflow && !surface.rendererItem
    }
    SourceScrollBar {
        orientation: Qt.Horizontal
        x: viewport.x; y: viewport.y + viewport.height
        width: viewport.width; height: 10
        visible: surface.horizontalOverflow && !surface.rendererItem
    }
    Component {
        id: plainText
        Text {
            id: plainLeaf
            text: surface.displayText
            textFormat: Text.PlainText
            readonly property real sourceLineHeight: text.length > 0 && lineCount > 0 ? implicitHeight / lineCount : metrics.height
            readonly property real sourceHeight: height
            readonly property real sourceWidth: contentWidth
            function geometry(point, position) {
                const input = {text: text, font: font, width: width, wrap: surface.wrapEnabled, alignment: surface.textAlignment}
                if (point !== null) input.point = point
                else input.position = position
                return surface.coordinator.plainCodeGeometry(input)
            }
            function sourcePosition(x, y) { return geometry(Qt.point(x, y), 0).position }
            function sourceRectangle(position) { return geometry(null, position).rect }
            wrapMode: surface.wrapEnabled ? Text.Wrap : Text.NoWrap
            horizontalAlignment: surface.textAlignment
            elide: Text.ElideNone
            font: surface.style.codeBlockFont
            color: surface.style.codeBlockColor
            height: text.length === 0 ? surface.lineHeight : implicitHeight
        }
    }
    Component {
        id: selectedText
        SelectableText {
            coordinator: surface.coordinator
            highlightColor: surface.searchHighlightColor
            currentHighlightColor: surface.currentSearchHighlightColor
            id: leaf
            projectionOffset: surface.projectionOffset
            projectionLength: surface.projectionLength
            text: surface.displayText
            foregroundRanges: syntaxRequest.ranges
            wrapEnabled: surface.wrapEnabled
            readonly property real sourceLineHeight: sourceLines.length ? sourceLines[0].height : metrics.height
            readonly property real sourceHeight: logicalHeight
            readonly property real sourceWidth: logicalWidth
            readonly property var visibleSourceLines: sourceLines.filter(function(line) { return line.top + line.height >= viewport.contentY && line.top <= viewport.contentY + viewport.height })
            function sourcePosition(x, y) { return cursorAt(x - leaf.x, y - leaf.y) }
            function sourceRectangle(position) { return sourceRectangleAt(position) }
            onLayoutChanged: surface.restorePosition()
            layoutWidth: contents.width
            horizontalAlignment: surface.textAlignment
            font: surface.style.codeBlockFont
            color: surface.style.codeBlockColor
            selectionColor: surface.style.selectionBackgroundColor
            implicitHeight: logicalHeight
        }
    }
}
