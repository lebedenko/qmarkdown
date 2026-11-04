import QtQuick
import QMarkdown 1.18
Rectangle {
    width: 900; height: 600; color: "white"
    property alias shift: flick.contentY
    property alias aligned: flick.pixelAligned
    Flickable {
        id: flick; anchors.fill: parent; contentHeight: md.height + 800; clip: true
        MarkdownView {
            id: md; x: 24.5; y: 24; width: 841; selectionEnabled: true; gfmEnabled: true
            markdown: "## Heading\n\n---\n\n| A | B | C |\n|---|---|---|\n" + "| One | Two | Three |\n".repeat(20)
            style: MarkdownStyle {
                preset: MarkdownStyle.PolishedLight
                bodyFont.pointSize: 11
                table.borderWidth: 1; table.borderColor: "red"
                table.headerBackground: "white"; table.bodyBackground: "white"; table.alternateRowBackground: "white"
                headings.h2.separatorColor: "red"; thematicBreakColor: "red"
            }
        }
    }
}
