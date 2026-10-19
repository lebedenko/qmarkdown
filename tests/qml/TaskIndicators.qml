import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material as MaterialStyle
import QMarkdown 1.10
import "../../examples/viewer" as Viewer

ApplicationWindow {
    id: window
    width: 600
    height: 300
    visible: true
    property real artworkScale: 1
    property bool dark: false
    property bool replacement: false
    MaterialStyle.Material.theme: dark ? MaterialStyle.Material.Dark : MaterialStyle.Material.Light
    palette.window: dark ? "#202124" : "#ffffff"
    palette.windowText: dark ? "#eeeeee" : "#202124"
    palette.text: dark ? "#eeeeee" : "#202124"
    palette.base: dark ? "#202124" : "#ffffff"
    palette.button: dark ? "#404247" : "#eeeeee"
    palette.buttonText: dark ? "#eeeeee" : "#202124"
    palette.highlight: "#608ce0"
    color: palette.window
    Column {
        x: 20; y: 10; spacing: 10
        Text { text: "Ordinary Controls checkboxes"; color: window.palette.windowText }
        Row {
            CheckBox { objectName: "ordinaryChecked"; checked: true; text: "Completed" }
            CheckBox { checked: false; text: "Pending" }
        }
        Text { text: "Host indicators"; color: window.palette.windowText }
        MarkdownView {
            objectName: "hostView"
            width: 540
            taskListsEnabled: true
            taskIndicatorsReplaceBullets: window.replacement
            markdown: "- [x] Completed\n- [ ] Pending"
            style.bodyColor: window.palette.windowText
            taskIndicator: Component { Viewer.TaskIndicator { hostPalette: window.palette; artworkScale: window.artworkScale } }
        }
        Text { text: "Library fallback"; color: window.palette.windowText }
        MarkdownView {
            width: 540
            taskListsEnabled: true
            taskIndicatorsReplaceBullets: window.replacement
            markdown: "- [x] Completed\n- [ ] Pending"
            style.bodyColor: window.palette.windowText
        }
    }
}
