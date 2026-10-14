pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private
import "private/Layout.js" as Layout

Item {
    id: actionHost
    required property var view
    required property ViewState coordinator
    required property string codeText
    property Component actions: actionHost.view.codeBlockActions
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
        const source = actionHost.view.markdown
        const component = actions
        const created = actions.createObject(actionHost, {
            codeText: text,
            requestCopy: function() {
                if (validity.active && actionHost.view.width > 0 && actionHost.view.markdown === source
                    && actionHost.view.codeBlockActions === component)
                    actionHost.view.codeBlockCopyRequested(text)
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
    Component.onCompleted: { actionHost.coordinator.registerSelectionItem(actionHost, ViewState.ActionSelection); ready = true; rebuild() }
    Component.onDestruction: { actionHost.coordinator.unregisterSelectionItem(actionHost); clearAction() }
}
