pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
import QMarkdown.Private
import "private/Layout.js" as Layout

Column {
    id: table
    objectName: "markdownTable"
    required property MarkdownStyle style
    required property ViewState coordinator
    required property Component sequenceComponent
    required property var childBlocks
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
                    SelectionRegistration {
                        coordinator: table.coordinator
                        item: tableCell
                        kind: ViewState.TableCellSelection
                    }
                    width: cells.count > 0 ? tableRow.width / cells.count : 0
                    x: index * width
                    height: tableRow.height
                    readonly property font cellFont: tableRow.header ? table.coordinator.boldFont(table.style.bodyFont) : table.style.bodyFont
                    readonly property real edge: Math.min(Layout.metric(table.style.table.borderWidth), width / 2)
                    readonly property real padX: Math.min(Layout.metric(table.style.table.horizontalPadding), Math.max(0, (width - 1 - 2 * edge) / 2))
                    readonly property real padY: Layout.metric(table.style.table.verticalPadding)
                    implicitHeight: Math.max(cellMetrics.height, cellContents.item ? (cellContents.item as Item).height : 0) + 2 * (padY + edge)
                    color: tableRow.header ? table.style.table.headerBackground : table.style.table.bodyBackground
                    border.color: table.style.table.borderColor
                    border.width: edge
                    FontMetrics { id: cellMetrics; font: tableCell.cellFont }
                    Loader {
                        id: cellContents
                        x: tableCell.edge + tableCell.padX
                        y: tableCell.edge + tableCell.padY
                        width: Math.max(0, tableCell.width - 2 * x)
                        sourceComponent: table.sequenceComponent
                        onLoaded: {
                            const loaded = item as BlockSequence
                            if (!loaded) {
                                console.warn("QMarkdown: TableBlock sequenceComponent must create a BlockSequence")
                                return
                            }
                            loaded.model = Qt.binding(function() { return tableCell.childBlocks })
                            loaded.style = Qt.binding(function() { return table.style })
                            loaded.bodyFont = Qt.binding(function() { return tableCell.cellFont })
                            loaded.bodyColor = Qt.binding(function() {
                                return tableRow.header ? table.style.table.headerForeground : table.style.bodyColor
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
