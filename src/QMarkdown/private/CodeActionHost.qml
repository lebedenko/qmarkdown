pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private
import "private/Layout.js" as Layout

Item {
    id: actionHost
    required property string sourceIdentity
    required property bool viewportAvailable
    signal copyRequested(string text)
    required property ViewState coordinator
    required property string language
    required property string infoString
    required property bool fenced
    required property string codeText
    required property Component actions
    property Item actionItem: null
    property var token: null
    property bool ready: false
    height: actionItem ? Layout.metric(actionItem.implicitHeight) : 0

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
        const source = actionHost.sourceIdentity
        const component = actions
        const metadata = coordinator.codeLanguageMetadata(fenced ? language : "")
        const created = coordinator.createHostComponent(actions, actionHost, {
            language: fenced ? language : "",
            languageId: metadata.languageId,
            languageName: metadata.languageName,
            infoString: infoString,
            fenced: fenced,
            codeText: text,
            requestCopy: function() {
                if (validity.active && actionHost.viewportAvailable && actionHost.sourceIdentity === source
                    && actionHost.actions === component)
                    actionHost.copyRequested(text)
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
    onLanguageChanged: rebuild()
    onInfoStringChanged: rebuild()
    onFencedChanged: rebuild()
    onSourceIdentityChanged: rebuild()
    SelectionRegistration {
        coordinator: actionHost.coordinator
        item: actionHost
        kind: ViewState.ActionSelection
    }
    Component.onCompleted: { ready = true; rebuild() }
    Component.onDestruction: { clearAction() }
}
