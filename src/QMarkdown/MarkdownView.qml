pragma ComponentBehavior: Bound
import QtQuick
import Qt.labs.qmlmodels
import QMarkdown.Private

Item {
    id: root
    property alias markdown: viewState.markdown
    property alias style: viewState.style
    readonly property real contentHeight: width > 0 ? blocks.height : 0
    implicitWidth: 0
    implicitHeight: contentHeight

    ViewState { id: viewState }
    Column {
        id: blocks
        width: Math.max(0, root.width)
        spacing: Number.isFinite(viewState.style.blockSpacing) ? Math.max(0, viewState.style.blockSpacing) : 8
        Repeater {
            model: root.width > 0 ? viewState.blocks : null
            delegate: DelegateChooser {
                role: "renderKind"
                DelegateChoice {
                    roleValue: 3
                    delegate: Rectangle {
                        width: blocks.width
                        height: Number.isFinite(viewState.style.thematicBreakThickness)
                            ? Math.max(0, viewState.style.thematicBreakThickness) : 1
                        color: viewState.style.thematicBreakColor
                    }
                }
                DelegateChoice {
                    roleValue: 2
                    delegate: Text {
                        id: codeBlock
                        required property string blockText
                        width: blocks.width
                        text: blockText.endsWith("\n") ? blockText.slice(0, -1) : blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideNone
                        font: viewState.style.codeBlockFont
                        color: viewState.style.codeBlockColor
                        height: text.length === 0 ? codeMetrics.height : implicitHeight
                        FontMetrics { id: codeMetrics; font: codeBlock.font }
                    }
                }
                DelegateChoice {
                    roleValue: 0
                    delegate: Text {
                        id: blockItem
                        required property string blockText
                        required property int headingLevel
                        width: blocks.width
                        text: blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideNone
                        font: headingLevel === 1 ? viewState.style.h1Font
                            : headingLevel === 2 ? viewState.style.h2Font
                            : headingLevel === 3 ? viewState.style.h3Font
                            : headingLevel === 4 ? viewState.style.h4Font
                            : headingLevel === 5 ? viewState.style.h5Font
                            : headingLevel === 6 ? viewState.style.h6Font : viewState.style.bodyFont
                        color: headingLevel === 1 ? viewState.style.h1Color
                            : headingLevel === 2 ? viewState.style.h2Color
                            : headingLevel === 3 ? viewState.style.h3Color
                            : headingLevel === 4 ? viewState.style.h4Color
                            : headingLevel === 5 ? viewState.style.h5Color
                            : headingLevel === 6 ? viewState.style.h6Color : viewState.style.bodyColor
                        height: blockText.length === 0 && headingLevel > 0 ? metrics.height : implicitHeight
                        FontMetrics { id: metrics; font: blockItem.font }
                    }
                }
                DelegateChoice {
                    roleValue: 1
                    delegate: Item {
                        id: formattedBlock
                        required property string blockText
                        required property int headingLevel
                        required property var formatRanges
                        width: blocks.width
                        height: painted.logicalHeight
                        FormattedText {
                            id: painted
                            text: formattedBlock.blockText
                            formatRanges: formattedBlock.formatRanges
                            layoutWidth: formattedBlock.width
                            codeFont: viewState.style.inlineCodeFont
                            font: formattedBlock.headingLevel === 1 ? viewState.style.h1Font
                                : formattedBlock.headingLevel === 2 ? viewState.style.h2Font
                                : formattedBlock.headingLevel === 3 ? viewState.style.h3Font
                                : formattedBlock.headingLevel === 4 ? viewState.style.h4Font
                                : formattedBlock.headingLevel === 5 ? viewState.style.h5Font
                                : formattedBlock.headingLevel === 6 ? viewState.style.h6Font : viewState.style.bodyFont
                            color: formattedBlock.headingLevel === 1 ? viewState.style.h1Color
                                : formattedBlock.headingLevel === 2 ? viewState.style.h2Color
                                : formattedBlock.headingLevel === 3 ? viewState.style.h3Color
                                : formattedBlock.headingLevel === 4 ? viewState.style.h4Color
                                : formattedBlock.headingLevel === 5 ? viewState.style.h5Color
                                : formattedBlock.headingLevel === 6 ? viewState.style.h6Color : viewState.style.bodyColor
                        }
                    }
                }
            }
        }
    }
}
