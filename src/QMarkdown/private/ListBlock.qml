pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown

Column {
    id: list
    required property MarkdownStyle style
    required property Component sequenceComponent

    required property var childBlocks
    required property bool tightList
    required property var markers
    readonly property real itemGap: tightList ? 0
        : (Number.isFinite(list.style.blockSpacing) ? Math.max(0, list.style.blockSpacing)
        : 8)
    spacing: itemGap
    FontMetrics {
        id: markerMetrics
        font: list.style.bodyFont
    }
    readonly property real markerWidth: {
        // Invokable measurements need an explicit font dependency.
        const fontDependency = markerMetrics.font
        return markers.reduce(function (w, marker) {
            return Math.max(w, markerMetrics.advanceWidth(marker))
        }, 0)
    }
    readonly property real gutter: Math.min(Math.max(0, width - 1), Math.max(Number.isFinite(list.style.listIndent) ? Math.max(0, list.style.listIndent)
        : 24, markerWidth + 8))
    Repeater {
        model: list.childBlocks
        delegate: Item {
            id: row
            required property int index
            required property var childBlocks
            required property int taskState
            width: list.width
            readonly property real taskSize: markerMetrics.height * 0.7
            readonly property real contentInset: Math.min(Math.max(0, width - 1), list.gutter + (taskState > 0 ? taskSize + 8
                : 0))
            height: Math.max(markerMetrics.height, itemLoader.item ? (itemLoader.item as Item).height : 0)
            Text {
                objectName: "listMarker"
                text: list.markers[row.index]
                width: Math.max(0, list.gutter - 8)
                horizontalAlignment: Text.AlignRight
                font: list.style.bodyFont
                color: list.style.bodyColor
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
                    border.color: list.style.bodyColor
                    radius: 2
                    Rectangle {
                        visible: taskIndicator.checked
                        x: parent.width * 0.22
                        y: parent.height * 0.5
                        width: parent.width * 0.3
                        height: Math.max(1, parent.height * 0.1)
                        color: list.style.bodyColor
                        rotation: 45
                        transformOrigin: Item.Left
                    }
                    Rectangle {
                        visible: taskIndicator.checked
                        x: parent.width * 0.43
                        y: parent.height * 0.71
                        width: parent.width * 0.5
                        height: Math.max(1, parent.height * 0.1)
                        color: list.style.bodyColor
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
                sourceComponent: list.sequenceComponent
                onLoaded: {
                    const loaded = item as BlockSequence
                    loaded.model = Qt.binding(function () {
                        return row.childBlocks
                    })
                    loaded.style = Qt.binding(function () {
                        return list.style
                    })
                    loaded.gap = Qt.binding(function () {
                        return list.itemGap
                    })
                }
            }
        }
    }
}
