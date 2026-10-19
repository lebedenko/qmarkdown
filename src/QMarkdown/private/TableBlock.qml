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
    readonly property int mode: style.table.borderMode
    readonly property real lineWidth: Math.min(Layout.metric(style.table.borderWidth), Math.max(0, width / Math.max(2, 2 * columnCount)))
    readonly property int columnCount: childBlocks.length > 0 ? childBlocks[0].childBlocks.length : 0
    Repeater {
        id: rows
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
            readonly property real topLine: table.mode === MarkdownStyle.TableBorderGrid || (table.mode === MarkdownStyle.TableBorderRows && index > 0) ? table.lineWidth : 0
            readonly property real bottomLine: (table.mode === MarkdownStyle.TableBorderGrid && index === rows.count - 1) || (table.mode === MarkdownStyle.TableBorderHeaderOnly && header) ? table.lineWidth : 0
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
                    readonly property real edge: table.mode === MarkdownStyle.TableBorderNone ? 0 : table.lineWidth
                    readonly property real padX: Math.min(Layout.metric(table.style.table.horizontalPadding), Math.max(0, (width - 1 - 2 * edge) / 2))
                    readonly property real padY: Layout.metric(table.style.table.verticalPadding)
                    implicitHeight: Math.max(cellMetrics.height, cellContents.item ? (cellContents.item as Item).height : 0) + 2 * (padY + edge)
                    color: tableRow.header ? table.style.table.headerBackground : (tableRow.index % 2 === 0 ? table.style.table.alternateRowBackground : table.style.table.bodyBackground)
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
            Item {
                anchors.fill: parent
                z: 1
                Rectangle {
                    objectName: "tableHorizontalLine"
                    width: tableRow.width; height: tableRow.topLine
                    color: table.style.table.borderColor
                }
                Rectangle {
                    objectName: "tableBottomLine"
                    y: tableRow.height - height
                    width: tableRow.width; height: tableRow.bottomLine
                    color: table.style.table.borderColor
                }
                Repeater {
                    model: table.mode === MarkdownStyle.TableBorderGrid ? cells.count + 1 : 0
                    delegate: Rectangle {
                        required property int index
                        objectName: "tableVerticalLine"
                        x: index === cells.count ? tableRow.width - width : Math.max(0, index * tableRow.width / cells.count - (index > 0 ? width / 2 : 0))
                        y: tableRow.topLine
                        width: table.lineWidth
                        height: Math.max(0, tableRow.height - tableRow.topLine - tableRow.bottomLine)
                        color: table.style.table.borderColor
                    }
                }
            }
        }
    }
}
