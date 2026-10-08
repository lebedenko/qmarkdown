pragma ComponentBehavior: Bound
import QtQuick
import Qt.labs.qmlmodels
import QMarkdown.Private

Item {
    id: root
    signal linkActivated(string destination)
    property alias baseUrl: viewState.baseUrl
    property alias resourcePolicy: viewState.resourcePolicy
    property alias markdown: viewState.markdown
    property alias style: viewState.style
    readonly property real contentHeight: width > 0 ? blocks.height : 0
    implicitWidth: 0
    implicitHeight: contentHeight

    ViewState { id: viewState }
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
        property var style: viewState.style
        property real gap: Number.isFinite(style.blockSpacing) ? Math.max(0, style.blockSpacing) : 8

        spacing: gap
        Repeater {
            model: sequence.width > 0 ? sequence.model : null
            delegate: DelegateChooser {
                role: "renderKind"
                DelegateChoice {
                    roleValue: 9
                    delegate: Loader {
                        id: segments
                        required property var childBlocks
                        width: sequence.width
                        sourceComponent: sequenceComponent
                        onLoaded: {
                            const loaded = item as BlockSequence
                            loaded.model = Qt.binding(function() { return segments.childBlocks })
                            loaded.style = Qt.binding(function() { return sequence.style })
                            loaded.gap = Qt.binding(function() { return sequence.gap })
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
                        width: sequence.width
                        height: imagePaint.implicitHeight
                        ImageItem {
                            id: imagePaint
                            objectName: "markdownImage"
                            image: imageRow.imageData
                            width: Math.min(imageRow.width, imagePaint.naturalWidth)
                            height: implicitHeight
                            HoverHandler { cursorShape: imageRow.imageLinked ? Qt.PointingHandCursor : Qt.ArrowCursor }
                            TapHandler {
                                acceptedButtons: Qt.LeftButton
                                gesturePolicy: TapHandler.DragThreshold
                                enabled: imageRow.imageLinked
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
                                width: list.width
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
                                Loader {
                                    id: itemLoader
                                    x: list.gutter
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
                    delegate: Text {
                        id: codeBlock
                        required property string blockText
                        width: sequence.width
                        text: blockText.endsWith("\n") ? blockText.slice(0, -1) : blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideNone
                        font: sequence.style.codeBlockFont
                        color: sequence.style.codeBlockColor
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
                        width: sequence.width
                        text: blockText
                        textFormat: Text.PlainText
                        wrapMode: Text.Wrap
                        horizontalAlignment: Text.AlignLeft
                        elide: Text.ElideNone
                        font: headingLevel === 1 ? sequence.style.h1Font
                            : headingLevel === 2 ? sequence.style.h2Font
                            : headingLevel === 3 ? sequence.style.h3Font
                            : headingLevel === 4 ? sequence.style.h4Font
                            : headingLevel === 5 ? sequence.style.h5Font
                            : headingLevel === 6 ? sequence.style.h6Font : sequence.style.bodyFont
                        color: headingLevel === 1 ? sequence.style.h1Color
                            : headingLevel === 2 ? sequence.style.h2Color
                            : headingLevel === 3 ? sequence.style.h3Color
                            : headingLevel === 4 ? sequence.style.h4Color
                            : headingLevel === 5 ? sequence.style.h5Color
                            : headingLevel === 6 ? sequence.style.h6Color : sequence.style.bodyColor
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
                        required property var linkSpans
                        required property var formatRanges
                        width: sequence.width
                        height: painted.logicalHeight
                        FormattedText {
                            id: painted
                            text: formattedBlock.blockText
                            formatRanges: formattedBlock.formatRanges
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
                            font: formattedBlock.headingLevel === 1 ? sequence.style.h1Font
                                : formattedBlock.headingLevel === 2 ? sequence.style.h2Font
                                : formattedBlock.headingLevel === 3 ? sequence.style.h3Font
                                : formattedBlock.headingLevel === 4 ? sequence.style.h4Font
                                : formattedBlock.headingLevel === 5 ? sequence.style.h5Font
                                : formattedBlock.headingLevel === 6 ? sequence.style.h6Font : sequence.style.bodyFont
                            color: formattedBlock.headingLevel === 1 ? sequence.style.h1Color
                                : formattedBlock.headingLevel === 2 ? sequence.style.h2Color
                                : formattedBlock.headingLevel === 3 ? sequence.style.h3Color
                                : formattedBlock.headingLevel === 4 ? sequence.style.h4Color
                                : formattedBlock.headingLevel === 5 ? sequence.style.h5Color
                                : formattedBlock.headingLevel === 6 ? sequence.style.h6Color : sequence.style.bodyColor
                        }
                    }
                }
            }
        }
    }
}
