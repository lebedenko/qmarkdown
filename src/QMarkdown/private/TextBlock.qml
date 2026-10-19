pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
import QMarkdown.Private
import "private/Layout.js" as Layout

Item {
    id: textBlock
    required property string blockText
    required property int headingLevel
    required property int selectionOffset
    required property int selectionLength
    required property var formatRanges
    required property var linkSpans
    required property var codeSpans
    required property int presentationKind
    required property MarkdownStyle style
    required property ViewState coordinator
    required property color searchHighlightColor
    required property color currentSearchHighlightColor
    property font bodyFont: style.bodyFont
    property color bodyColor: style.bodyColor
    property int textAlignment: Qt.AlignLeft
    property bool suppressHeadingSeparators: false
    readonly property bool selectionEnabled: coordinator.selectionEnabled
    signal linkActivated(string destination)
    property real minimumHeight: 0
    height: Math.max(minimumHeight, contents.item ? (contents.item as Item).height : 0)
    Loader {
        id: contents
        width: textBlock.width
        sourceComponent: textBlock.presentationKind === ViewState.PlainText ? nativeText
            : textBlock.presentationKind === ViewState.SelectedText ? selectedTextComponent
            : formattedText
    }
    Component {
        id: nativeText
        Text {
            id: blockItem
            property int selectionOffset: textBlock.selectionOffset
            property int selectionLength: textBlock.selectionLength
            property int projectionOffset: selectionOffset
            property int projectionLength: selectionLength
            SelectionRegistration {
                coordinator: textBlock.coordinator
                item: blockItem
                offset: blockItem.projectionOffset
                length: blockItem.projectionLength
            }
            property string blockText: textBlock.blockText
            property int headingLevel: textBlock.headingLevel
            width: textBlock.width
            text: blockText
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            horizontalAlignment: textBlock.textAlignment
            elide: Text.ElideNone
            font: headingLevel === 1 ? textBlock.style.h1Font
                : headingLevel === 2 ? textBlock.style.h2Font
                : headingLevel === 3 ? textBlock.style.h3Font
                : headingLevel === 4 ? textBlock.style.h4Font
                : headingLevel === 5 ? textBlock.style.h5Font
                : headingLevel === 6 ? textBlock.style.h6Font
                : textBlock.bodyFont
            color: headingLevel === 1 ? textBlock.style.h1Color
                : headingLevel === 2 ? textBlock.style.h2Color
                : headingLevel === 3 ? textBlock.style.h3Color
                : headingLevel === 4 ? textBlock.style.h4Color
                : headingLevel === 5 ? textBlock.style.h5Color
                : headingLevel === 6 ? textBlock.style.h6Color
                : textBlock.bodyColor
            readonly property real textHeight: blockText.length === 0 && headingLevel > 0 ? metrics.height
                : implicitHeight
            height: textHeight + plainRule.decorationSpacing + plainRule.height
            HeadingSeparator {
                id: plainRule
                width: blockItem.width
                y: blockItem.textHeight + decorationSpacing
                headingStyle: Layout.headingStyle(textBlock.style, blockItem.headingLevel)
                suppressed: textBlock.suppressHeadingSeparators
            }
            FontMetrics {
                id: metrics
                font: blockItem.font
            }
        }
    }
    Component {
        id: selectedTextComponent
        Item {
            id: plainBlock
            property string blockText: textBlock.blockText
            property int headingLevel: textBlock.headingLevel
            property int selectionOffset: textBlock.selectionOffset
            property int selectionLength: textBlock.selectionLength
            width: textBlock.width
            height: selectedText.logicalHeight + selectedRule.decorationSpacing + selectedRule.height
            SelectableText {
                id: selectedText
                coordinator: textBlock.coordinator
                highlightColor: textBlock.searchHighlightColor
                currentHighlightColor: textBlock.currentSearchHighlightColor
                text: plainBlock.blockText
                layoutWidth: plainBlock.width
                projectionOffset: plainBlock.selectionOffset
                projectionLength: plainBlock.selectionLength
                horizontalAlignment: textBlock.textAlignment
                font: plainBlock.headingLevel > 0 ? Layout.headingStyle(textBlock.style, plainBlock.headingLevel).font
                    : textBlock.bodyFont
                color: plainBlock.headingLevel > 0 ? Layout.headingStyle(textBlock.style, plainBlock.headingLevel).color
                    : textBlock.bodyColor
                selectionColor: textBlock.style.selectionBackgroundColor
            }
            HeadingSeparator {
                id: selectedRule
                width: plainBlock.width
                y: selectedText.logicalHeight + decorationSpacing
                headingStyle: Layout.headingStyle(textBlock.style, plainBlock.headingLevel)
                suppressed: textBlock.suppressHeadingSeparators
            }
        }
    }
    Component {
        id: formattedText
        Item {
            id: formattedBlock
            property string blockText: textBlock.blockText
            property int headingLevel: textBlock.headingLevel
            property var linkSpans: textBlock.linkSpans
            property var codeSpans: textBlock.codeSpans
            property var formatRanges: textBlock.formatRanges
            property int selectionOffset: textBlock.selectionOffset
            property int selectionLength: textBlock.selectionLength
            width: textBlock.width
            height: painted.logicalHeight + formattedRule.decorationSpacing + formattedRule.height
            HeadingSeparator {
                id: formattedRule
                width: formattedBlock.width
                y: painted.logicalHeight + decorationSpacing
                headingStyle: Layout.headingStyle(textBlock.style, formattedBlock.headingLevel)
                suppressed: textBlock.suppressHeadingSeparators
            }
            SelectableText {
                id: painted
                coordinator: textBlock.coordinator
                highlightColor: textBlock.searchHighlightColor
                currentHighlightColor: textBlock.currentSearchHighlightColor
                projectionOffset: formattedBlock.selectionOffset
                projectionLength: formattedBlock.selectionLength
                selectionColor: textBlock.style.selectionBackgroundColor
                horizontalAlignment: textBlock.textAlignment
                text: formattedBlock.blockText
                formatRanges: formattedBlock.formatRanges
                codeSpans: formattedBlock.codeSpans
                linkSpans: formattedBlock.linkSpans
                linkColor: textBlock.style.linkColor
                linkUnderline: textBlock.style.linkUnderline
                Connections {
                    target: textBlock.coordinator
                    function onPresentationReset() {
                        painted.pressedLink = -1
                    }
                }
                property int hoveredLink: -1
                property int pressedLink: -1
                function updateHover() {
                    const position = painted.mapFromItem(null, hover.point.scenePosition)
                    hoveredLink = hover.hovered ? painted.linkAt(position.x, position.y) : -1
                }
                onLayoutChanged: updateHover()
                onTextChanged: {
                    pressedLink = -1
                    updateHover()
                }
                onLinkSpansChanged: {
                    pressedLink = -1
                    updateHover()
                }
                HoverHandler {
                    id: hover
                    cursorShape: painted.hoveredLink >= 0 ? Qt.PointingHandCursor : Qt.ArrowCursor
                    onHoveredChanged: painted.updateHover()
                    onPointChanged: painted.updateHover()
                }
                TapHandler {
                    id: tap
                    acceptedDevices: textBlock.selectionEnabled ? PointerDevice.TouchScreen | PointerDevice.Stylus
                        : PointerDevice.AllDevices
                    acceptedButtons: Qt.LeftButton
                    gesturePolicy: TapHandler.DragThreshold
                    onPressedChanged: {
                        if (pressed)
                            painted.pressedLink = painted.linkAt(point.position.x, point.position.y)
                    }
                    onCanceled: painted.pressedLink = -1
                    onTapped: function (eventPoint, button) {
                        const hit = painted.linkAt(eventPoint.position.x, eventPoint.position.y)
                        if (hit >= 0 && hit === painted.pressedLink)
                            textBlock.linkActivated(painted.linkDestination(hit))
                        painted.pressedLink = -1
                    }
                }
                layoutWidth: formattedBlock.width
                codeFont: textBlock.style.inlineCodeFont
                codeForeground: textBlock.style.inlineCode.foreground
                codeBackground: textBlock.style.inlineCode.backgroundColor
                codeCornerRadius: textBlock.style.inlineCode.cornerRadius
                codeHorizontalPadding: textBlock.style.inlineCode.horizontalPadding
                codeVerticalPadding: textBlock.style.inlineCode.verticalPadding
                font: formattedBlock.headingLevel === 1 ? textBlock.style.h1Font
                    : formattedBlock.headingLevel === 2 ? textBlock.style.h2Font
                    : formattedBlock.headingLevel === 3 ? textBlock.style.h3Font
                    : formattedBlock.headingLevel === 4 ? textBlock.style.h4Font
                    : formattedBlock.headingLevel === 5 ? textBlock.style.h5Font
                    : formattedBlock.headingLevel === 6 ? textBlock.style.h6Font
                    : textBlock.bodyFont
                color: formattedBlock.headingLevel === 1 ? textBlock.style.h1Color
                    : formattedBlock.headingLevel === 2 ? textBlock.style.h2Color
                    : formattedBlock.headingLevel === 3 ? textBlock.style.h3Color
                    : formattedBlock.headingLevel === 4 ? textBlock.style.h4Color
                    : formattedBlock.headingLevel === 5 ? textBlock.style.h5Color
                    : formattedBlock.headingLevel === 6 ? textBlock.style.h6Color
                    : textBlock.bodyColor
            }
        }
    }
}
