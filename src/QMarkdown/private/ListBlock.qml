pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown

Column {
    id: list
    required property MarkdownStyle style
    required property Component taskIndicator
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
            readonly property real taskSize: Math.max(16, Math.round(markerMetrics.height * 0.85))
            readonly property real contentInset: Math.min(Math.max(0, width - 1), list.gutter + (taskState > 0 ? indicatorLoader.preferredWidth + 8
                : 0))
            readonly property real firstLineHeight: Math.max(markerMetrics.height, indicatorLoader.preferredHeight)
            height: Math.max(firstLineHeight, itemLoader.y + (itemLoader.item ? (itemLoader.item as Item).height : 0))
            Text {
                objectName: "listMarker"
                y: (row.firstLineHeight - markerMetrics.height) / 2
                text: list.markers[row.index]
                width: Math.max(0, list.gutter - 8)
                horizontalAlignment: Text.AlignRight
                font: list.style.bodyFont
                color: list.style.bodyColor
                textFormat: Text.PlainText
            }
            Loader {
                id: indicatorLoader
                active: row.taskState > 0
                x: list.gutter
                y: (row.firstLineHeight - preferredHeight) / 2
                width: Math.min(preferredWidth, Math.max(0, row.contentInset - x))
                height: preferredHeight
                readonly property real preferredWidth: item ? (item as Item).implicitWidth : 0
                readonly property real preferredHeight: item ? (item as Item).implicitHeight : 0
                sourceComponent: Component {
                    TaskIndicatorHost {
                        taskState: row.taskState
                        markdownStyle: list.style
                        indicator: list.taskIndicator
                        defaultSize: row.taskSize
                    }
                }
            }
            Loader {
                id: itemLoader
                objectName: "listItemContent"
                y: (row.firstLineHeight - markerMetrics.height) / 2
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
