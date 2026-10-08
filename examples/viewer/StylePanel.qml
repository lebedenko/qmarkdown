pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QMarkdown

ColumnLayout {
    id: panel
    required property MarkdownStyle targetStyle
    required property color defaultTextColor
    required property color previewBackgroundColor
    readonly property bool darkSurface: previewBackgroundColor.hslLightness < 0.5
    readonly property var roles: ["body", "h1", "h2", "h3", "h4", "h5", "h6", "inlineCode", "codeBlock"]
    property int selectedRole: 0
    property int revision: 0
    readonly property string role: roles[selectedRole]
    readonly property string colorRole: role === "inlineCode" ? "body" : role
    MarkdownStyle { id: defaults }
    function refresh() { revision++ }
    function editFont(field, value) {
        let font = targetStyle[role + "Font"]
        font[field] = value
        targetStyle[role + "Font"] = font
        refresh()
    }
    function isValidColor(value) {
        try { return Qt.colorEqual(value, value) }
        catch (error) { return false }
    }
    function applyColor(value) {
        if (!isValidColor(value)) return false
        targetStyle[colorRole + "Color"] = value
        refresh()
        return true
    }
    function resetStyle() {
        for (const name of roles) {
            targetStyle[name + "Font"] = defaults[name + "Font"]
            if (name !== "inlineCode")
                targetStyle[name + "Color"] = Qt.binding(function() { return panel.defaultTextColor })
        }
        targetStyle.blockSpacing = defaults.blockSpacing
        refresh()
    }
    function applyPreset(alternate) {
        resetStyle()
        if (alternate) {
            targetStyle.bodyFont.pixelSize = 20
            targetStyle.bodyColor = Qt.binding(function() { return panel.darkSurface ? "#82cba7" : "#194c39" })
            targetStyle.h1Font.pixelSize = 40
            for (let i = 1; i <= 6; ++i)
                targetStyle["h" + i + "Color"] = Qt.binding(function() { return panel.darkSurface ? "#d8a0e5" : "#743a86" })
            targetStyle.codeBlockFont.pixelSize = 22
            targetStyle.codeBlockColor = Qt.binding(function() { return panel.darkSurface ? "#8db9f2" : "#305b9c" })
            targetStyle.inlineCodeFont.family = "serif"
            targetStyle.blockSpacing = 16
        }
        refresh()
    }
    RowLayout {
        Button { text: "Neutral"; objectName: "neutralButton"; onClicked: panel.applyPreset(false) }
        Button { text: "Alternate"; objectName: "alternateButton"; onClicked: panel.applyPreset(true) }
        Button { text: "Reset style"; objectName: "resetButton"; onClicked: panel.resetStyle() }
        Item { Layout.fillWidth: true }
    }
    RowLayout {
        Label { text: "Text role" }
        ComboBox {
            objectName: "roleSelector"
            model: ["Body", "H1", "H2", "H3", "H4", "H5", "H6", "Inline code", "Fenced code"]
            currentIndex: panel.selectedRole
            onActivated: panel.selectedRole = currentIndex
        }
        Label { text: "Family" }
        TextField {
            objectName: "familyField"
            Layout.fillWidth: true
            text: { panel.revision; return panel.targetStyle[panel.role + "Font"].family }
            onEditingFinished: if (text.trim().length) panel.editFont("family", text.trim())
        }
        Label { text: "Size (px)" }
        SpinBox {
            objectName: "sizeField"
            from: 8; to: 72; editable: true
            value: { panel.revision; const size = panel.targetStyle[panel.role + "Font"].pixelSize; return size > 0 ? size : panel.targetStyle.bodyFont.pixelSize }
            onValueModified: panel.editFont("pixelSize", value)
        }
    }
    RowLayout {
        Label { text: panel.role === "inlineCode" ? "Color (body)" : "Color" }
        TextField {
            id: colorField
            objectName: "colorField"
            Layout.preferredWidth: 180
            text: { panel.revision; return panel.targetStyle[panel.colorRole + "Color"].toString() }
            readonly property bool validColor: panel.isValidColor(text)
            color: validColor ? palette.text : "#b02020"
            onEditingFinished: panel.applyColor(text)
        }
        Label { text: colorField.validColor ? "" : "Invalid color"; color: "#b02020" }
        Label { text: "Block spacing" }
        SpinBox {
            objectName: "spacingField"
            from: 0; to: 48; editable: true
            value: panel.targetStyle.blockSpacing
            onValueModified: panel.targetStyle.blockSpacing = value
        }
        Label {
            Layout.fillWidth: true
            wrapMode: Text.WordWrap
            text: panel.role === "inlineCode" ? "Size inherits until edited; heading code uses heading color." : ""
        }
    }
}
