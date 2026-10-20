pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private

Item {
    id: root
    property alias codeBlockWrapEnabled: viewState.codeBlockWrapEnabled
    property alias codeBlockLineNumbersVisible: viewState.codeBlockLineNumbersVisible
    property alias codeBlockHeightLimitEnabled: viewState.codeBlockHeightLimitEnabled
    property alias codeBlockViewportHeight: viewState.codeBlockViewportHeight
    property alias codeBlockHeightRatio: viewState.codeBlockHeightRatio
    property alias codeBlockMinimumLines: viewState.codeBlockMinimumLines
    property alias codeBlockMaximumLines: viewState.codeBlockMaximumLines
    property alias codeBlockScrollBar: viewState.codeBlockScrollBar
    property alias syntaxHighlightingEnabled: viewState.syntaxHighlightingEnabled
    readonly property bool syntaxHighlightingAvailable: viewState.syntaxHighlightingAvailable
    property alias syntaxTheme: viewState.syntaxTheme
    property bool keyboardNavigationEnabled: false
    Binding {
        target: viewState.linkController
        property: "enabled"
        value: root.keyboardNavigationEnabled
    }
    property color keyboardFocusColor: style.linkColor
    Binding {
        target: viewState.linkController
        property: "color"
        value: root.keyboardFocusColor
    }
    property alias searchActive: viewState.searchActive
    property alias searchQuery: viewState.searchQuery
    property alias searchCaseSensitive: viewState.searchCaseSensitive
    readonly property int searchResultCount: viewState.searchResultCount
    readonly property int currentSearchResult: viewState.currentSearchResult
    readonly property var headings: viewState.headings
    property color searchHighlightColor: "#70ffff00"
    property color currentSearchHighlightColor: "#b0ff9900"
    signal navigationRequested(rect target)
    function searchNext() { return viewState.searchNext() }
    function searchPrevious() { return viewState.searchPrevious() }
    function navigateToHeading(index: int): bool { return viewState.navigateToHeading(index) }
    function navigateToFragment(fragment: string): bool { return viewState.navigateToFragment(fragment) }
    onWidthChanged: viewState.refreshNavigation()
    onContentHeightChanged: viewState.refreshNavigation()
    signal copyRequested(string text)
    property alias selectionViewport: viewState.selectionViewport
    signal selectionScrollRequested(real deltaY)
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
        if (!root.selectionEnabled || (!root.activeFocus && !viewState.linkController.hasLinkFocus())) return
        if (event.matches(StandardKey.SelectAll)) root.selectAll()
        else if (event.matches(StandardKey.Copy)) root.requestCopy()
        else if (event.key === Qt.Key_Escape) root.clearSelection()
        else return
        event.accepted = true
    }
    // Wayland can label mouse events as TouchPad; accept both pointer devices.
    // Reserve document mouse taps after the passive observer excludes actions.
    // Qt 6.8 Flickable otherwise retains the grab and filters drag updates.
    TapHandler {
        enabled: root.selectionEnabled && !selectionGesture.blocked
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedButtons: Qt.LeftButton
        gesturePolicy: TapHandler.WithinBounds
    }
    PointHandler {
        enabled: root.selectionEnabled
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedButtons: Qt.LeftButton
        onActiveChanged: if (!active) selectionGesture.blocked = false
    }
    TapHandler {
        id: selectionTap
        enabled: root.selectionEnabled
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
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
            if (link !== null && link !== undefined && link === selectionGesture.pressedLink) viewState.finishSelectionTap(root, eventPoint.position)
        }
    }
    DragHandler {
        id: selectionDrag
        grabPermissions: PointerHandler.CanTakeOverFromAnything
        enabled: root.selectionEnabled && !selectionGesture.blocked
        acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad
        acceptedButtons: Qt.LeftButton
        target: null
        onActiveChanged: {
            if (active) {
                selectionGesture.dragged = true
                viewState.dragSelection(root, centroid.position)
            } else viewState.endSelectionDrag()
        }
        onCanceled: viewState.cancelSelectionGesture()
        onCentroidChanged: if (active) viewState.dragSelection(root, centroid.position)
    }
    Connections {
        target: viewState
        function onNavigationRequested(target) { root.navigationRequested(target) }
        function onSelectionScrollRequested(deltaY) { root.selectionScrollRequested(deltaY) }
        function onSelectionLinkActivated(destination) { root.linkActivated(destination) }
    }
    signal linkActivated(string destination)
    signal codeBlockCopyRequested(string text)
    property Component taskIndicator: null
    property bool taskIndicatorsReplaceBullets: false
    property Component codeBlockActions: null
    property alias inlineMathEnabled: viewState.inlineMathEnabled
    property alias mathEnabled: viewState.mathEnabled
    readonly property bool mathAvailable: viewState.mathAvailable
    property var fencedBlockRenderers: ({})
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

    ViewState {
        id: viewState
        navigationView: root
        onCopyRequested: function(text) { root.copyRequested(text) }
        onPresentationReset: selectionGesture.pressedLink = null
    }
    Loader {
        id: blocks
        width: Math.max(0, root.width)
        sourceComponent: recursiveSequence
        onLoaded: {
            const loaded = item as BlockSequence
            loaded.model = Qt.binding(function() { return root.width > 0 ? viewState.blocks : null })
        }
    }
    Component {
        id: recursiveSequence
        BlockSequence {
            sequenceComponent: recursiveSequence
            coordinator: viewState
            style: viewState.style
            taskIndicator: root.taskIndicator
            taskIndicatorsReplaceBullets: root.taskIndicatorsReplaceBullets
            codeBlockActions: root.codeBlockActions
            fencedBlockRenderers: root.fencedBlockRenderers
            sourceIdentity: root.markdown
            viewportAvailable: root.width > 0
            searchHighlightColor: root.searchHighlightColor
            currentSearchHighlightColor: root.currentSearchHighlightColor
            onLinkActivated: function(destination) { root.linkActivated(destination) }
            onCodeBlockCopyRequested: function(text) { root.codeBlockCopyRequested(text) }
        }
    }
}
