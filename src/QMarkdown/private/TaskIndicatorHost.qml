pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
import QMarkdown.Private

TaskIndicatorHostItem {
    id: host
    objectName: "taskIndicator"
    required property int taskState
    readonly property bool checked: taskState === 2
    required property MarkdownStyle markdownStyle
    required property Component indicator
    required property real defaultSize
    property Item customItem: null
    property var token: null
    property bool ready: false
    implicitWidth: customItem && Number.isFinite(customItem.implicitWidth) && customItem.implicitWidth > 0
        ? customItem.implicitWidth : defaultSize
    implicitHeight: customItem && Number.isFinite(customItem.implicitHeight) && customItem.implicitHeight > 0
        ? customItem.implicitHeight : defaultSize
    clip: true
    Accessible.role: Accessible.CheckBox
    Accessible.name: checked ? qsTr("Completed task") : qsTr("Incomplete task")
    Accessible.description: qsTr("Read-only Markdown task")
    Accessible.checkable: true
    Accessible.checked: checked
    Accessible.focusable: false
    Accessible.readOnly: true

    function clearIndicator() {
        if (token) token.active = false
        token = null
        if (customItem) {
            customItem.visible = false
            customItem.destroy()
            customItem = null
        }
    }
    function rebuild() {
        clearIndicator()
        if (!ready || !indicator) return
        const validity = { active: true }
        token = validity
        const created = indicator.createObject(host, {
            checked: Qt.binding(function() { return host.checked }),
            markdownStyle: Qt.binding(function() { return host.markdownStyle })
        })
        const item = created as Item
        if (!item) {
            if (created) created.destroy()
            console.warn("QMarkdown: taskIndicator must create an Item with checked and markdownStyle properties")
            return
        }
        // Host application code may replace the component during creation.
        if (!validity.active) {
            item.destroy()
            return
        }
        customItem = item
        item.width = Qt.binding(function() { return host.implicitWidth })
        item.height = Qt.binding(function() { return host.implicitHeight })
    }
    TaskIndicatorItem {
        objectName: "taskBox"
        visible: !host.customItem
        width: host.defaultSize
        height: host.defaultSize
        checked: host.checked
        color: host.markdownStyle.bodyColor
        Accessible.ignored: true
    }
    onIndicatorChanged: { clearIndicator(); Qt.callLater(rebuild) }
    Component.onCompleted: { ready = true; Qt.callLater(rebuild) }
    Component.onDestruction: clearIndicator()
}
