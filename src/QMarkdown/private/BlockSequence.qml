pragma ComponentBehavior: Bound
import QtQuick
import Qt.labs.qmlmodels
import QMarkdown
import QMarkdown.Private
import "private/Layout.js" as Layout

Column {
    id: sequence
    objectName: "blockSequence"
    required property ViewState coordinator
    required property Component codeBlockActions
    required property var fencedBlockRenderers
    required property string sourceIdentity
    required property bool viewportAvailable
    required property color searchHighlightColor
    required property color currentSearchHighlightColor
    readonly property bool selectionEnabled: coordinator.selectionEnabled
    readonly property bool searchActive: coordinator.searchActive
    signal linkActivated(string destination)
    signal codeBlockCopyRequested(string text)
    required property Component sequenceComponent
    property var model: null
    property bool suppressHeadingSeparators: false
    required property MarkdownStyle style
    property font bodyFont: style.bodyFont
    property color bodyColor: style.bodyColor
    property int textAlignment: Qt.AlignLeft
    property real gap: Number.isFinite(style.blockSpacing) ? Math.max(0, style.blockSpacing) : 8

    spacing: gap
    Repeater {
        model: sequence.width > 0 ? sequence.model : null
        delegate: DelegateChooser {
            role: "renderKind"
            DelegateChoice {
                roleValue: ViewState.Table
                delegate: TableBlock {
                    width: sequence.width
                    style: sequence.style
                    coordinator: sequence.coordinator
                    sequenceComponent: sequence.sequenceComponent
                }
            }
            DelegateChoice {
                roleValue: ViewState.Segments
                delegate: Item {
                    id: segments
                    required property var childBlocks
                    required property int headingLevel
                    width: sequence.width
                    height: segmentRows.height + segmentRule.decorationSpacing + segmentRule.height
                    Loader {
                        id: segmentRows
                        width: segments.width
                        sourceComponent: sequence.sequenceComponent
                        onLoaded: {
                            const loaded = item as BlockSequence
                            loaded.model = Qt.binding(function () {
                                return segments.childBlocks
                            })
                            loaded.style = Qt.binding(function () {
                                return sequence.style
                            })
                            loaded.gap = Qt.binding(function () {
                                return sequence.gap
                            })
                            loaded.bodyFont = Qt.binding(function () {
                                return sequence.bodyFont
                            })
                            loaded.bodyColor = Qt.binding(function () {
                                return sequence.bodyColor
                            })
                            loaded.textAlignment = Qt.binding(function () {
                                return sequence.textAlignment
                            })
                            loaded.suppressHeadingSeparators = true
                        }
                    }
                    HeadingSeparator {
                        id: segmentRule
                        width: segments.width
                        y: segmentRows.height + decorationSpacing
                        headingStyle: Layout.headingStyle(sequence.style, segments.headingLevel)
                        suppressed: sequence.suppressHeadingSeparators
                    }
                }
            }
            DelegateChoice {
                roleValue: ViewState.Image
                delegate: ImageBlock {
                    width: sequence.width
                    style: sequence.style
                    coordinator: sequence.coordinator
                    textAlignment: sequence.textAlignment
                    searchHighlightColor: sequence.searchHighlightColor
                    currentSearchHighlightColor: sequence.currentSearchHighlightColor
                    onLinkActivated: function (destination) {
                        sequence.linkActivated(destination)
                    }
                }
            }
            DelegateChoice {
                roleValue: ViewState.List
                delegate: ListBlock {
                    width: sequence.width
                    style: sequence.style
                    sequenceComponent: sequence.sequenceComponent
                }
            }
            DelegateChoice {
                roleValue: ViewState.Quote
                delegate: Item {
                    id: quote
                    required property var childBlocks
                    width: sequence.width
                    readonly property real thickness: Number.isFinite(sequence.style.quoteRuleThickness) ? Math.max(0, sequence.style.quoteRuleThickness)
                        : 2
                    readonly property real inset: Math.min(Math.max(0, width - 1), Math.max(thickness + 8, Number.isFinite(sequence.style.quoteIndent) ? Math.max(0, sequence.style.quoteIndent)
                        : 16))
                    height: Math.max(quoteMetrics.height, quoteLoader.item ? (quoteLoader.item as Item).height : 0)
                    FontMetrics {
                        id: quoteMetrics
                        font: sequence.style.bodyFont
                    }
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
                        sourceComponent: sequence.sequenceComponent
                        onLoaded: {
                            const loaded = item as BlockSequence
                            loaded.model = Qt.binding(function () {
                                return quote.childBlocks
                            })
                            loaded.style = Qt.binding(function () {
                                return sequence.style
                            })
                        }
                    }
                }
            }
            DelegateChoice {
                roleValue: ViewState.HtmlBlock
                delegate: Item {
                    id: htmlBlock
                    required property string blockText
                    required property int selectionOffset
                    required property int selectionLength
                    width: sequence.width
                    height: htmlLoader.item ? (htmlLoader.item as Item).height : 0
                    Loader {
                        id: htmlLoader
                        width: htmlBlock.width
                        sourceComponent: sequence.selectionEnabled || sequence.searchActive ? selectedHtml
                            : plainHtml
                    }
                    Component {
                        id: plainHtml
                        Text {
                            text: htmlBlock.blockText.endsWith("\n") ? htmlBlock.blockText.slice(0, -1)
                                : htmlBlock.blockText
                            textFormat: Text.PlainText
                            wrapMode: Text.Wrap
                            font: sequence.style.bodyFont
                            color: sequence.style.bodyColor
                        }
                    }
                    Component {
                        id: selectedHtml
                        SelectableText {
                            coordinator: sequence.coordinator
                            highlightColor: sequence.searchHighlightColor
                            currentHighlightColor: sequence.currentSearchHighlightColor
                            text: htmlBlock.blockText.endsWith("\n") ? htmlBlock.blockText.slice(0, -1)
                                : htmlBlock.blockText
                            layoutWidth: htmlBlock.width
                            projectionOffset: htmlBlock.selectionOffset
                            projectionLength: htmlBlock.selectionLength
                            font: sequence.style.bodyFont
                            color: sequence.style.bodyColor
                            selectionColor: sequence.style.selectionBackgroundColor
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: ViewState.ThematicBreak
                delegate: Rectangle {
                    width: sequence.width
                    height: Number.isFinite(sequence.style.thematicBreakThickness) ? Math.max(0, sequence.style.thematicBreakThickness)
                        : 1
                    color: sequence.style.thematicBreakColor
                }
            }
            DelegateChoice {
                roleValue: ViewState.CodeBlock
                delegate: CodeSurface {
                    required property string codeLanguage
                    language: codeLanguage
                    required property string codeInfo
                    required property bool codeFenced
                    infoString: codeInfo
                    fenced: codeFenced
                    required property string blockText
                    required property int selectionOffset
                    required property int selectionLength
                    width: sequence.width
                    codeText: blockText
                    style: sequence.style
                    coordinator: sequence.coordinator
                    actions: sequence.codeBlockActions
                    rendererRegistry: sequence.fencedBlockRenderers
                    sourceIdentity: sequence.sourceIdentity
                    viewportAvailable: sequence.viewportAvailable
                    searchActive: sequence.searchActive
                    searchHighlightColor: sequence.searchHighlightColor
                    currentSearchHighlightColor: sequence.currentSearchHighlightColor
                    onCopyRequested: function (text) {
                        sequence.codeBlockCopyRequested(text)
                    }
                    selected: sequence.selectionEnabled
                    projectionOffset: selectionOffset
                    projectionLength: selectionLength
                    textAlignment: sequence.textAlignment
                }
            }

            DelegateChoice {
                roleValue: ViewState.PlainText
                delegate: TextBlock {
                    width: sequence.width
                    presentationKind: sequence.selectionEnabled || sequence.searchActive ? ViewState.SelectedText
                        : ViewState.PlainText
                    style: sequence.style
                    coordinator: sequence.coordinator
                    bodyFont: sequence.bodyFont
                    bodyColor: sequence.bodyColor
                    textAlignment: sequence.textAlignment
                    suppressHeadingSeparators: sequence.suppressHeadingSeparators
                    searchHighlightColor: sequence.searchHighlightColor
                    currentSearchHighlightColor: sequence.currentSearchHighlightColor
                    onLinkActivated: function (destination) {
                        sequence.linkActivated(destination)
                    }
                }
            }

            DelegateChoice {
                roleValue: ViewState.FormattedText
                delegate: TextBlock {
                    width: sequence.width
                    presentationKind: ViewState.FormattedText
                    style: sequence.style
                    coordinator: sequence.coordinator
                    bodyFont: sequence.bodyFont
                    bodyColor: sequence.bodyColor
                    textAlignment: sequence.textAlignment
                    suppressHeadingSeparators: sequence.suppressHeadingSeparators
                    searchHighlightColor: sequence.searchHighlightColor
                    currentSearchHighlightColor: sequence.currentSearchHighlightColor
                    onLinkActivated: function (destination) {
                        sequence.linkActivated(destination)
                    }
                }
            }
        }
    }
}
