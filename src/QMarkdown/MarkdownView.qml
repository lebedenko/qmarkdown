pragma ComponentBehavior: Bound
import QtQuick
import Qt.labs.qmlmodels
import QMarkdown.Private

Item {
    id: root
    signal copyRequested(string text)
    property alias selectionEnabled: viewState.selectionEnabled
    readonly property string selectedText: viewState.selectedText
    readonly property int selectionStart: viewState.selectionStart
    readonly property int selectionEnd: viewState.selectionEnd
    function selectAll() { viewState.selectAll() }
    function clearSelection() { viewState.clearSelection() }
    function requestCopy() { viewState.requestCopy() }
    QtObject {
        id: selectionGesture
        property bool blocked: false
        property bool dragged: false
        property bool shift: false
        property var pressedLink: null
    }
    Keys.onPressed: function(event) {
        if (!root.selectionEnabled || !root.activeFocus) return
        if (event.matches(StandardKey.SelectAll)) root.selectAll()
        else if (event.matches(StandardKey.Copy)) root.requestCopy()
        else if (event.key === Qt.Key_Escape) root.clearSelection()
        else return
        event.accepted = true
    }
    // Reserve document mouse taps after the passive observer excludes actions.
    // Qt 6.8 Flickable otherwise retains the grab and filters drag updates.
    TapHandler {
        enabled: root.selectionEnabled && !selectionGesture.blocked
        acceptedDevices: PointerDevice.Mouse
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.WithinBounds
    }
    PointHandler {
        enabled: root.selectionEnabled
        acceptedDevices: PointerDevice.Mouse
        acceptedButtons: Qt.LeftButton
        onActiveChanged: if (!active) selectionGesture.blocked = false
    }
    TapHandler {
        id: selectionTap
        enabled: root.selectionEnabled
        acceptedDevices: PointerDevice.Mouse
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.DragThreshold
        onPressedChanged: {
            if (pressed) {
                selectionGesture.dragged = false
                selectionGesture.shift = (point.modifiers & Qt.ShiftModifier) !== 0
                selectionGesture.blocked = !viewState.beginSelection(root, point.position, selectionGesture.shift)
                selectionGesture.pressedLink = viewState.selectionLink(root, point.position)
                if (!selectionGesture.blocked) root.forceActiveFocus(Qt.MouseFocusReason)
            }
        }
        onTapped: function(eventPoint, button) {
            if (selectionGesture.blocked || selectionGesture.dragged || selectionGesture.shift) return
            const link = viewState.selectionLink(root, eventPoint.position)
            if (link !== null && link !== undefined && link === selectionGesture.pressedLink) root.linkActivated(link)
        }
    }
    DragHandler {
        id: selectionDrag
        grabPermissions: PointerHandler.CanTakeOverFromAnything
        enabled: root.selectionEnabled && !selectionGesture.blocked
        acceptedDevices: PointerDevice.Mouse
        acceptedButtons: Qt.LeftButton
        target: null
        onActiveChanged: {
            if (active) {
                selectionGesture.dragged = true
                viewState.extendSelection(root, centroid.position)
            }
        }
        onCentroidChanged: if (active) viewState.extendSelection(root, centroid.position)
    }
    signal linkActivated(string destination)
    signal codeBlockCopyRequested(string text)
    property Component codeBlockActions: null
    property alias baseUrl: viewState.baseUrl
    property alias resourcePolicy: viewState.resourcePolicy
    property alias markdown: viewState.markdown
    property alias extendedAutolinksEnabled: viewState.extendedAutolinksEnabled
    property alias strikethroughEnabled: viewState.strikethroughEnabled
    property alias tablesEnabled: viewState.tablesEnabled
    property alias tagFilterEnabled: viewState.tagFilterEnabled
    property alias gfmEnabled: viewState.gfmEnabled
    property alias taskListsEnabled: viewState.taskListsEnabled
    property alias style: viewState.style
    readonly property real contentHeight: width > 0 ? blocks.height : 0
    implicitWidth: 0
    implicitHeight: contentHeight

    function metric(value: real): real {
        return Number.isFinite(value) ? Math.max(0, value) : 0
    }
    function headingStyle(style: var, level: int): var {
        return level > 0 ? style.headings["h" + level] : null
    }
    component SelectableText: FormattedText {
        id: selectable
        property int projectionOffset: 0
        property int projectionLength: 0
        selectionStart: root.selectionEnabled ? Math.max(0, root.selectionStart - projectionOffset) : 0
        selectionEnd: root.selectionEnabled ? Math.max(0, root.selectionEnd - projectionOffset) : 0
        implicitHeight: logicalHeight
        Component.onCompleted: viewState.registerSelectionItem(selectable, 0)
        Component.onDestruction: viewState.unregisterSelectionItem(selectable)
    }
    component HeadingSeparator: Rectangle {
        required property var headingStyle
        property bool suppressed: false
        readonly property bool enabledDecoration: !suppressed && headingStyle !== null
            && headingStyle.separatorVisible
        readonly property real decorationSpacing: enabledDecoration ? root.metric(headingStyle.separatorSpacing) : 0
        height: enabledDecoration ? root.metric(headingStyle.separatorThickness) : 0
        color: headingStyle !== null ? headingStyle.separatorColor : "transparent"
        objectName: "headingSeparator"
    }

    component CodeActionHost: Item {
        id: actionHost
        required property string codeText
        property Component actions: root.codeBlockActions
        property Item actionItem: null
        property var token: null
        property bool ready: false
        height: actionItem ? root.metric(actionItem.implicitHeight) : 0

        function clearAction() {
            if (token) token.active = false
            token = null
            if (actionItem) {
                actionItem.visible = false
                actionItem.destroy()
                actionItem = null
            }
        }
        function rebuild() {
            clearAction()
            if (!ready || !actions) return
            const validity = { active: true }
            token = validity
            const text = codeText
            const source = root.markdown
            const component = actions
            const created = actions.createObject(actionHost, {
                codeText: text,
                requestCopy: function() {
                    if (validity.active && root.width > 0 && root.markdown === source
                        && root.codeBlockActions === component)
                        root.codeBlockCopyRequested(text)
                }
            })
            const item = created as Item
            if (!item) {
                validity.active = false
                if (created) created.destroy()
                console.warn("QMarkdown: codeBlockActions must create an Item with codeText and requestCopy properties")
                return
            }
            // A host may replace the component synchronously during creation.
            if (!validity.active) {
                item.destroy()
                return
            }
            actionItem = item
            item.width = Qt.binding(function() { return actionHost.width })
            item.height = Qt.binding(function() { return actionHost.height })
        }
        onActionsChanged: rebuild()
        onCodeTextChanged: rebuild()
        Component.onCompleted: { viewState.registerSelectionItem(actionHost, 3); ready = true; rebuild() }
        Component.onDestruction: { viewState.unregisterSelectionItem(actionHost); clearAction() }
    }

    ViewState {
        id: viewState
        onCopyRequested: function(text) { root.copyRequested(text) }
        onPresentationReset: selectionGesture.pressedLink = null
    }
    Loader {
        id: blocks
        width: Math.max(0, root.width)
        sourceComponent: sequenceComponent
        onLoaded: {
            const loaded = item as BlockSequence
            loaded.model = Qt.binding(function() { return root.width > 0 ? viewState.blocks : null })
        }
    }
    Component {
        id: sequenceComponent
        BlockSequence {}
    }
    component BlockSequence: Column {
        id: sequence
        property var model: null
        property bool suppressHeadingSeparators: false
        property var style: viewState.style
        property font bodyFont: style.bodyFont
        property color bodyColor: style.bodyColor
        property int textAlignment: Qt.AlignLeft
        property real gap: Number.isFinite(style.blockSpacing) ? Math.max(0, style.blockSpacing) : 8

        spacing: gap
        Repeater {
            model: sequence.width > 0 ? sequence.model : null
            delegate: DelegateChooser {
                role: root.selectionEnabled ? "selectionKind" : "renderKind"
                DelegateChoice {
                    roleValue: 10
                    delegate: Column {
                        id: table
                        objectName: "markdownTable"
                        required property var childBlocks
                        width: sequence.width
                        spacing: 0
                        Repeater {
                            model: table.childBlocks
                            delegate: Item {
                                id: tableRow
                                objectName: "tableRow"
                                required property int index
                                required property var childBlocks
                                readonly property bool header: index === 0
                                width: table.width
                                height: {
                                    let tallest = 0
                                    for (let i = 0; i < cells.count; ++i) {
                                        const cell = cells.itemAt(i)
                                        if (cell) tallest = Math.max(tallest, cell.implicitHeight)
                                    }
                                    return tallest
                                }
                                Repeater {
                                    id: cells
                                    model: tableRow.childBlocks
                                    delegate: Rectangle {
                                        id: tableCell
                                        objectName: "tableCell"
                                        required property int index
                                        required property var childBlocks
                                        required property int tableAlignment
                                        Component.onCompleted: viewState.registerSelectionItem(tableCell, 2)
                                        Component.onDestruction: viewState.unregisterSelectionItem(tableCell)
                                        width: cells.count > 0 ? tableRow.width / cells.count : 0
                                        x: index * width
                                        height: tableRow.height
                                        readonly property font cellFont: tableRow.header ? viewState.boldFont(sequence.style.bodyFont) : sequence.style.bodyFont
                                        readonly property real edge: Math.min(root.metric(sequence.style.table.borderWidth), width / 2)
                                        readonly property real padX: Math.min(root.metric(sequence.style.table.horizontalPadding), Math.max(0, (width - 1 - 2 * edge) / 2))
                                        readonly property real padY: root.metric(sequence.style.table.verticalPadding)
                                        implicitHeight: Math.max(cellMetrics.height, cellContents.item ? (cellContents.item as Item).height : 0) + 2 * (padY + edge)
                                        color: tableRow.header ? sequence.style.table.headerBackground : sequence.style.table.bodyBackground
                                        border.color: sequence.style.table.borderColor
                                        border.width: edge
                                        FontMetrics { id: cellMetrics; font: tableCell.cellFont }
                                        Loader {
                                            id: cellContents
                                            x: tableCell.edge + tableCell.padX
                                            y: tableCell.edge + tableCell.padY
                                            width: Math.max(0, tableCell.width - 2 * x)
                                            sourceComponent: sequenceComponent
                                            onLoaded: {
                                                const loaded = item as BlockSequence
                                                loaded.model = Qt.binding(function() { return tableCell.childBlocks })
                                                loaded.style = Qt.binding(function() { return sequence.style })
                                                loaded.bodyFont = Qt.binding(function() { return tableCell.cellFont })
                                                loaded.bodyColor = Qt.binding(function() {
                                                    return tableRow.header ? sequence.style.table.headerForeground : sequence.style.bodyColor
                                                })
                                                loaded.textAlignment = Qt.binding(function() {
                                                    return tableCell.tableAlignment === 2 ? Qt.AlignHCenter
                                                        : tableCell.tableAlignment === 3 ? Qt.AlignRight : Qt.AlignLeft
                                                })
                                                loaded.gap = 0
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 9
                    delegate: Item {
                        id: segments
                        required property var childBlocks
                        required property int headingLevel
                        width: sequence.width
                        height: segmentRows.height + segmentRule.decorationSpacing + segmentRule.height
                        Loader {
                            id: segmentRows
                            width: segments.width
                            sourceComponent: sequenceComponent
                            onLoaded: {
                                const loaded = item as BlockSequence
                                loaded.model = Qt.binding(function() { return segments.childBlocks })
                                loaded.style = Qt.binding(function() { return sequence.style })
                                loaded.gap = Qt.binding(function() { return sequence.gap })
                                loaded.bodyFont = Qt.binding(function() { return sequence.bodyFont })
                                loaded.bodyColor = Qt.binding(function() { return sequence.bodyColor })
                                loaded.textAlignment = Qt.binding(function() { return sequence.textAlignment })
                                loaded.suppressHeadingSeparators = true
                            }
                        }
                        HeadingSeparator {
                            id: segmentRule
                            width: segments.width
                            y: segmentRows.height + decorationSpacing
                            headingStyle: root.headingStyle(sequence.style, segments.headingLevel)
                            suppressed: sequence.suppressHeadingSeparators
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 8
                    delegate: Item {
                        id: imageRow
                        required property var imageData
                        required property string imageLink
                        required property bool imageLinked
                        required property int selectionOffset
                        required property int selectionLength
                        width: sequence.width
                        height: imagePaint.implicitHeight
                        ImageItem {
                            id: imagePaint
                            objectName: "markdownImage"
                            property int projectionOffset: imageRow.selectionOffset
                            property int projectionLength: imageRow.selectionLength
                            property bool imageLinked: imageRow.imageLinked
                            property string imageLink: imageRow.imageLink
                            Component.onCompleted: viewState.registerSelectionItem(imagePaint, 1)
                            Component.onDestruction: viewState.unregisterSelectionItem(imagePaint)
                            Rectangle {
                                anchors.fill: parent
                                color: sequence.style.selectionBackgroundColor
                                visible: root.selectionEnabled && root.selectionStart < root.selectionEnd
                                    && imageRow.selectionLength > 0 && root.selectionStart < imageRow.selectionOffset + imageRow.selectionLength
                                    && root.selectionEnd > imageRow.selectionOffset
                            }
                            image: imageRow.imageData
                            width: Math.min(imageRow.width, imagePaint.naturalWidth)
                            height: implicitHeight
                            x: sequence.textAlignment === Qt.AlignRight ? imageRow.width - width
                                : sequence.textAlignment === Qt.AlignHCenter ? (imageRow.width - width) / 2 : 0
                            HoverHandler { cursorShape: imageRow.imageLinked ? Qt.PointingHandCursor : Qt.ArrowCursor }
                            TapHandler {
                                acceptedButtons: Qt.LeftButton
                                gesturePolicy: TapHandler.DragThreshold
                                enabled: imageRow.imageLinked
                                acceptedDevices: root.selectionEnabled ? PointerDevice.TouchScreen | PointerDevice.TouchPad | PointerDevice.Stylus : PointerDevice.AllDevices
                                onTapped: function(eventPoint, button) {
                                    if (eventPoint.position.x >= 0 && eventPoint.position.x < imagePaint.width
                                        && eventPoint.position.y >= 0 && eventPoint.position.y < imagePaint.height)
                                        root.linkActivated(imageRow.imageLink)
                                }
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 4
                    delegate: Column {
                        id: list
                        required property var childBlocks
                        required property bool tightList
                        required property var markers
                        width: sequence.width
                        readonly property real itemGap: tightList ? 0
                            : (Number.isFinite(sequence.style.blockSpacing) ? Math.max(0, sequence.style.blockSpacing) : 8)
                        spacing: itemGap
                        FontMetrics { id: markerMetrics; font: sequence.style.bodyFont }
                        readonly property real markerWidth: {
                            // Invokable measurements need an explicit font dependency.
                            const fontDependency = markerMetrics.font
                            return markers.reduce(function(w, marker) { return Math.max(w, markerMetrics.advanceWidth(marker)) }, 0)
                        }
                        readonly property real gutter: Math.min(Math.max(0, width - 1), Math.max(
                            Number.isFinite(sequence.style.listIndent) ? Math.max(0, sequence.style.listIndent) : 24,
                            markerWidth + 8))
                        Repeater {
                            model: list.childBlocks
                            delegate: Item {
                                id: row
                                required property int index
                                required property var childBlocks
                                required property int taskState
                                width: list.width
                                readonly property real taskSize: markerMetrics.height * 0.7
                                readonly property real contentInset: Math.min(Math.max(0, width - 1),
                                    list.gutter + (taskState > 0 ? taskSize + 8 : 0))
                                height: Math.max(markerMetrics.height, itemLoader.item ? (itemLoader.item as Item).height : 0)
                                Text {
                                    objectName: "listMarker"
                                    text: list.markers[row.index]
                                    width: Math.max(0, list.gutter - 8)
                                    horizontalAlignment: Text.AlignRight
                                    font: sequence.style.bodyFont
                                    color: sequence.style.bodyColor
                                    textFormat: Text.PlainText
                                }
                                Item {
                                    id: taskIndicator
                                    objectName: "taskIndicator"
                                    visible: row.taskState > 0
                                    readonly property bool checked: row.taskState === 2
                                    x: list.gutter
                                    y: (markerMetrics.height - height) / 2
                                    width: Math.min(row.taskSize, Math.max(0, row.contentInset - x))
                                    height: row.taskSize
                                    clip: true
                                    Accessible.role: Accessible.CheckBox
                                    Accessible.name: checked ? qsTr("Completed task") : qsTr("Incomplete task")
                                    Accessible.description: qsTr("Read-only Markdown task")
                                    Accessible.checkable: true
                                    Accessible.checked: checked
                                    Accessible.focusable: false
                                    Accessible.readOnly: true
                                    Accessible.ignored: !visible
                                    Rectangle {
                                        objectName: "taskBox"
                                        width: row.taskSize
                                        height: row.taskSize
                                        color: "transparent"
                                        border.width: 1
                                        border.color: sequence.style.bodyColor
                                        radius: 2
                                        Rectangle {
                                            visible: taskIndicator.checked
                                            x: parent.width * 0.22
                                            y: parent.height * 0.5
                                            width: parent.width * 0.3
                                            height: Math.max(1, parent.height * 0.1)
                                            color: sequence.style.bodyColor
                                            rotation: 45
                                            transformOrigin: Item.Left
                                        }
                                        Rectangle {
                                            visible: taskIndicator.checked
                                            x: parent.width * 0.43
                                            y: parent.height * 0.71
                                            width: parent.width * 0.5
                                            height: Math.max(1, parent.height * 0.1)
                                            color: sequence.style.bodyColor
                                            rotation: -50
                                            transformOrigin: Item.Left
                                        }
                                    }
                                }
                                Loader {
                                    id: itemLoader
                                    objectName: "listItemContent"
                                    x: row.contentInset
                                    width: Math.max(0, row.width - x)
                                    sourceComponent: sequenceComponent
                                    onLoaded: {
                                        const loaded = item as BlockSequence
                                        loaded.model = Qt.binding(function() { return row.childBlocks })
                                        loaded.style = Qt.binding(function() { return sequence.style })
                                        loaded.gap = Qt.binding(function() { return list.itemGap })
                                    }
                                }
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 5
                    delegate: Item {
                        id: quote
                        required property var childBlocks
                        width: sequence.width
                        readonly property real thickness: Number.isFinite(sequence.style.quoteRuleThickness) ? Math.max(0, sequence.style.quoteRuleThickness) : 2
                        readonly property real inset: Math.min(Math.max(0, width - 1), Math.max(thickness + 8,
                            Number.isFinite(sequence.style.quoteIndent) ? Math.max(0, sequence.style.quoteIndent) : 16))
                        height: Math.max(quoteMetrics.height, quoteLoader.item ? (quoteLoader.item as Item).height : 0)
                        FontMetrics { id: quoteMetrics; font: sequence.style.bodyFont }
                        Rectangle {
                            objectName: "quoteRule"
                            width: Math.min(quote.thickness, quote.inset)
                            height: quote.height
                            color: sequence.style.quoteRuleColor
                        }
                        Loader {
                            id: quoteLoader
                            x: quote.inset
                            width: Math.max(0, quote.width - x)
                            sourceComponent: sequenceComponent
                            onLoaded: {
                                const loaded = item as BlockSequence
                                loaded.model = Qt.binding(function() { return quote.childBlocks })
                                loaded.style = Qt.binding(function() { return sequence.style })
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 7
                    delegate: Text {
                        required property string blockText
                        width: sequence.width
                        text: blockText.endsWith("\n") ? blockText.slice(0, -1) : blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        font: sequence.style.bodyFont
                        color: sequence.style.bodyColor
                    }
                }
                DelegateChoice {
                    roleValue: 13
                    delegate: Item {
                        id: htmlBlock
                        required property string blockText
                        required property int selectionOffset
                        required property int selectionLength
                        width: sequence.width
                        height: htmlLoader.item ? (htmlLoader.item as Item).height : 0
                        Loader {
                            id: htmlLoader
                            width: htmlBlock.width
                            sourceComponent: selectedHtml
                        }
                        Component {
                            id: selectedHtml
                            SelectableText {
                                text: htmlBlock.blockText.endsWith("\n") ? htmlBlock.blockText.slice(0, -1) : htmlBlock.blockText
                                layoutWidth: htmlBlock.width
                                projectionOffset: htmlBlock.selectionOffset
                                projectionLength: htmlBlock.selectionLength
                                font: sequence.style.bodyFont
                                color: sequence.style.bodyColor
                                selectionColor: sequence.style.selectionBackgroundColor
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 3
                    delegate: Rectangle {
                        width: sequence.width
                        height: Number.isFinite(sequence.style.thematicBreakThickness)
                            ? Math.max(0, sequence.style.thematicBreakThickness) : 1
                        color: sequence.style.thematicBreakColor
                    }
                }
                DelegateChoice {
                    roleValue: 2
                    delegate: Rectangle {
                        id: codeSurface
                        required property string blockText
                        width: sequence.width
                        readonly property real padX: Math.min(root.metric(sequence.style.codeBlock.horizontalPadding),
                            Math.max(0, (width - 1) / 2))
                        readonly property real padY: root.metric(sequence.style.codeBlock.verticalPadding)
                        height: codeActions.height + codeBlock.height + 2 * padY
                        color: sequence.style.codeBlock.backgroundColor
                        border.color: sequence.style.codeBlock.borderColor
                        border.width: root.metric(sequence.style.codeBlock.borderWidth)
                        radius: root.metric(sequence.style.codeBlock.cornerRadius)
                        objectName: "codeSurface"
                        CodeActionHost {
                            id: codeActions
                            x: codeSurface.padX
                            y: codeSurface.padY
                            width: Math.max(1, codeSurface.width - 2 * codeSurface.padX)
                            codeText: codeSurface.blockText
                        }
                        Text {
                            id: codeBlock
                            x: codeSurface.padX
                            y: codeSurface.padY + codeActions.height
                            width: Math.max(1, codeSurface.width - 2 * codeSurface.padX)
                            text: codeSurface.blockText.endsWith("\n") ? codeSurface.blockText.slice(0, -1) : codeSurface.blockText
                            textFormat: Text.PlainText
                            wrapMode: Text.Wrap
                            horizontalAlignment: sequence.textAlignment
                            elide: Text.ElideNone
                            font: sequence.style.codeBlockFont
                            color: sequence.style.codeBlockColor
                            height: text.length === 0 ? codeMetrics.height : implicitHeight
                            FontMetrics { id: codeMetrics; font: codeBlock.font }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 12
                    delegate: Rectangle {
                        id: selectedCodeSurface
                        required property string blockText
                        required property int selectionOffset
                        required property int selectionLength
                        width: sequence.width
                        readonly property real padX: Math.min(root.metric(sequence.style.codeBlock.horizontalPadding),
                            Math.max(0, (width - 1) / 2))
                        readonly property real padY: root.metric(sequence.style.codeBlock.verticalPadding)
                        height: selectedCodeActions.height + selectedCodeBlock.height + 2 * padY
                        color: sequence.style.codeBlock.backgroundColor
                        border.color: sequence.style.codeBlock.borderColor
                        border.width: root.metric(sequence.style.codeBlock.borderWidth)
                        radius: root.metric(sequence.style.codeBlock.cornerRadius)
                        objectName: "codeSurface"
                        CodeActionHost {
                            id: selectedCodeActions
                            x: selectedCodeSurface.padX
                            y: selectedCodeSurface.padY
                            width: Math.max(1, selectedCodeSurface.width - 2 * selectedCodeSurface.padX)
                            codeText: selectedCodeSurface.blockText
                        }
                        Loader {
                            id: selectedCodeBlock
                            x: selectedCodeSurface.padX
                            y: selectedCodeSurface.padY + selectedCodeActions.height
                            width: Math.max(1, selectedCodeSurface.width - 2 * selectedCodeSurface.padX)
                            sourceComponent: selectedCode
                        }
                        Component {
                            id: selectedCode
                            SelectableText {
                                text: selectedCodeSurface.blockText.endsWith("\n") ? selectedCodeSurface.blockText.slice(0, -1) : selectedCodeSurface.blockText
                                layoutWidth: selectedCodeBlock.width
                                projectionOffset: selectedCodeSurface.selectionOffset
                                projectionLength: selectedCodeSurface.selectionLength
                                horizontalAlignment: sequence.textAlignment
                                font: sequence.style.codeBlockFont
                                color: sequence.style.codeBlockColor
                                selectionColor: sequence.style.selectionBackgroundColor
                            }
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 0
                    delegate: Text {
                        id: blockItem
                        required property string blockText
                        required property int headingLevel
                        width: sequence.width
                        text: blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        horizontalAlignment: sequence.textAlignment
                        elide: Text.ElideNone
                        font: headingLevel === 1 ? sequence.style.h1Font
                            : headingLevel === 2 ? sequence.style.h2Font
                            : headingLevel === 3 ? sequence.style.h3Font
                            : headingLevel === 4 ? sequence.style.h4Font
                            : headingLevel === 5 ? sequence.style.h5Font
                            : headingLevel === 6 ? sequence.style.h6Font : sequence.bodyFont
                        color: headingLevel === 1 ? sequence.style.h1Color
                            : headingLevel === 2 ? sequence.style.h2Color
                            : headingLevel === 3 ? sequence.style.h3Color
                            : headingLevel === 4 ? sequence.style.h4Color
                            : headingLevel === 5 ? sequence.style.h5Color
                            : headingLevel === 6 ? sequence.style.h6Color : sequence.bodyColor
                        readonly property real textHeight: blockText.length === 0 && headingLevel > 0 ? metrics.height : implicitHeight
                        height: textHeight + plainRule.decorationSpacing + plainRule.height
                        HeadingSeparator {
                            id: plainRule
                            width: blockItem.width
                            y: blockItem.textHeight + decorationSpacing
                            headingStyle: root.headingStyle(sequence.style, blockItem.headingLevel)
                            suppressed: sequence.suppressHeadingSeparators
                        }
                        FontMetrics { id: metrics; font: blockItem.font }
                    }
                }
                DelegateChoice {
                    roleValue: 11
                    delegate: Item {
                        id: plainBlock
                        required property string blockText
                        required property int headingLevel
                        required property int selectionOffset
                        required property int selectionLength
                        width: sequence.width
                        height: selectedText.logicalHeight + selectedRule.decorationSpacing + selectedRule.height
                        SelectableText {
                            id: selectedText
                            text: plainBlock.blockText
                            layoutWidth: plainBlock.width
                            projectionOffset: plainBlock.selectionOffset
                            projectionLength: plainBlock.selectionLength
                            horizontalAlignment: sequence.textAlignment
                            font: plainBlock.headingLevel > 0 ? root.headingStyle(sequence.style, plainBlock.headingLevel).font : sequence.bodyFont
                            color: plainBlock.headingLevel > 0 ? root.headingStyle(sequence.style, plainBlock.headingLevel).color : sequence.bodyColor
                            selectionColor: sequence.style.selectionBackgroundColor
                        }
                        HeadingSeparator {
                            id: selectedRule
                            width: plainBlock.width
                            y: selectedText.logicalHeight + decorationSpacing
                            headingStyle: root.headingStyle(sequence.style, plainBlock.headingLevel)
                            suppressed: sequence.suppressHeadingSeparators
                        }
                    }
                }
                DelegateChoice {
                    roleValue: 1
                    delegate: Item {
                        id: formattedBlock
                        required property string blockText
                        required property int headingLevel
                        required property var linkSpans
                        required property var codeSpans
                        required property var formatRanges
                        required property int selectionOffset
                        required property int selectionLength
                        width: sequence.width
                        height: painted.logicalHeight + formattedRule.decorationSpacing + formattedRule.height
                        HeadingSeparator {
                            id: formattedRule
                            width: formattedBlock.width
                            y: painted.logicalHeight + decorationSpacing
                            headingStyle: root.headingStyle(sequence.style, formattedBlock.headingLevel)
                            suppressed: sequence.suppressHeadingSeparators
                        }
                        SelectableText {
                            id: painted
                            projectionOffset: formattedBlock.selectionOffset
                            projectionLength: formattedBlock.selectionLength
                            selectionColor: sequence.style.selectionBackgroundColor
                            horizontalAlignment: sequence.textAlignment
                            text: formattedBlock.blockText
                            formatRanges: formattedBlock.formatRanges
                            codeSpans: formattedBlock.codeSpans
                            linkSpans: formattedBlock.linkSpans
                            linkColor: sequence.style.linkColor
                            linkUnderline: sequence.style.linkUnderline
                            property int hoveredLink: -1
                            property int pressedLink: -1
                            function updateHover() {
                                const position = painted.mapFromItem(null, hover.point.scenePosition)
                                hoveredLink = hover.hovered ? painted.linkAt(position.x, position.y) : -1
                            }
                            onLayoutChanged: updateHover()
                            onTextChanged: { pressedLink = -1; updateHover() }
                            onLinkSpansChanged: { pressedLink = -1; updateHover() }
                            HoverHandler {
                                id: hover
                                cursorShape: painted.hoveredLink >= 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                                onHoveredChanged: painted.updateHover()
                                onPointChanged: painted.updateHover()
                            }
                            TapHandler {
                                id: tap
                                acceptedDevices: root.selectionEnabled ? PointerDevice.TouchScreen | PointerDevice.TouchPad | PointerDevice.Stylus : PointerDevice.AllDevices
                                acceptedButtons: Qt.LeftButton
                                gesturePolicy: TapHandler.DragThreshold
                                onPressedChanged: {
                                    if (pressed) painted.pressedLink = painted.linkAt(point.position.x, point.position.y)
                                }
                                onCanceled: painted.pressedLink = -1
                                onTapped: function(eventPoint, button) {
                                    const hit = painted.linkAt(eventPoint.position.x, eventPoint.position.y)
                                    if (hit >= 0 && hit === painted.pressedLink)
                                        root.linkActivated(painted.linkDestination(hit))
                                    painted.pressedLink = -1
                                }
                            }
                            layoutWidth: formattedBlock.width
                            codeFont: sequence.style.inlineCodeFont
                            codeForeground: sequence.style.inlineCode.foreground
                            codeBackground: sequence.style.inlineCode.backgroundColor
                            codeCornerRadius: sequence.style.inlineCode.cornerRadius
                            codeHorizontalPadding: sequence.style.inlineCode.horizontalPadding
                            codeVerticalPadding: sequence.style.inlineCode.verticalPadding
                            font: formattedBlock.headingLevel === 1 ? sequence.style.h1Font
                                : formattedBlock.headingLevel === 2 ? sequence.style.h2Font
                                : formattedBlock.headingLevel === 3 ? sequence.style.h3Font
                                : formattedBlock.headingLevel === 4 ? sequence.style.h4Font
                                : formattedBlock.headingLevel === 5 ? sequence.style.h5Font
                                : formattedBlock.headingLevel === 6 ? sequence.style.h6Font : sequence.bodyFont
                            color: formattedBlock.headingLevel === 1 ? sequence.style.h1Color
                                : formattedBlock.headingLevel === 2 ? sequence.style.h2Color
                                : formattedBlock.headingLevel === 3 ? sequence.style.h3Color
                                : formattedBlock.headingLevel === 4 ? sequence.style.h4Color
                                : formattedBlock.headingLevel === 5 ? sequence.style.h5Color
                                : formattedBlock.headingLevel === 6 ? sequence.style.h6Color : sequence.bodyColor
                        }
                    }
                }
            }
        }
    }
}
