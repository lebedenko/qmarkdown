pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QMarkdown
import QMarkdownViewer.Tools

ColumnLayout {
    id: panel
    required property MarkdownStyle targetStyle
    required property color defaultLinkColor
    required property color defaultTextColor
    required property color previewBackgroundColor
    readonly property bool darkSurface: previewBackgroundColor.hslLightness < 0.5
    readonly property var roles: ["body", "h1", "h2", "h3", "h4", "h5", "h6", "inlineCode", "codeBlock"]
    property int selectedRole: 0
    property int revision: 0
    readonly property string role: roles[selectedRole]
    readonly property string colorRole: role === "inlineCode" ? "body" : role
    // The preview shares this window and screen; Screen density is logical dots/mm.
    readonly property real logicalDpi: Screen.logicalPixelDensity * 25.4
    FontEditor { id: fontEditor }
    readonly property font effectiveFont: {
        panel.revision
        const font = targetStyle[role + "Font"]
        return role === "inlineCode" && !fontEditor.hasSize(font)
            ? targetStyle.bodyFont : font
    }
    readonly property bool pointUnit: fontEditor.points(effectiveFont)
    readonly property real fontSize: fontEditor.size(effectiveFont)
    MarkdownStyle { id: defaults }
    function changeUnit(points) {
        if (points === pointUnit) return
        const size = points ? fontSize * 72 / logicalDpi : Math.max(1, Math.round(fontSize * logicalDpi / 72))
        targetStyle[role + "Font"] = fontEditor.resized(targetStyle[role + "Font"], points, size)
        refresh()
    }
    function editSize(size) {
        targetStyle[role + "Font"] = fontEditor.resized(targetStyle[role + "Font"], pointUnit, size)
        refresh()
    }
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
    function applyRuleColor(value) {
        if (!isValidColor(value)) return false
        targetStyle.thematicBreakColor = value
        refresh()
        return true
    }
    function applyQuoteColor(value) {
        if (!isValidColor(value)) return false
        targetStyle.quoteRuleColor = value
        refresh()
        return true
    }
    function resetStyle() {
        for (const name of roles) {
            targetStyle[name + "Font"] = defaults[name + "Font"]
            if (name !== "inlineCode")
                targetStyle[name + "Color"] = Qt.binding(function() { return panel.defaultTextColor })
        }
        targetStyle.linkColor = Qt.binding(function() { return panel.defaultLinkColor })
        targetStyle.linkUnderline = defaults.linkUnderline
        targetStyle.thematicBreakColor = Qt.binding(function() { return panel.defaultTextColor })
        targetStyle.thematicBreakThickness = defaults.thematicBreakThickness
        targetStyle.quoteRuleColor = Qt.binding(function() { return panel.darkSurface ? "#a0a0a0" : "#707070" })
        targetStyle.quoteRuleThickness = defaults.quoteRuleThickness
        targetStyle.quoteIndent = defaults.quoteIndent
        targetStyle.listIndent = defaults.listIndent
        targetStyle.blockSpacing = defaults.blockSpacing
        refresh()
    }
    function applyPreset(alternate) {
        resetStyle()
        if (alternate) {
            targetStyle.linkColor = Qt.binding(function() { return panel.darkSurface ? "#8db9f2" : "#305b9c" })
            targetStyle.linkUnderline = false
            targetStyle.bodyFont = fontEditor.resized(targetStyle.bodyFont, false, 20)
            targetStyle.bodyColor = Qt.binding(function() { return panel.darkSurface ? "#82cba7" : "#194c39" })
            targetStyle.h1Font = fontEditor.resized(targetStyle.h1Font, false, 40)
            for (let i = 1; i <= 6; ++i)
                targetStyle["h" + i + "Color"] = Qt.binding(function() { return panel.darkSurface ? "#d8a0e5" : "#743a86" })
            targetStyle.codeBlockFont = fontEditor.resized(targetStyle.codeBlockFont, false, 22)
            targetStyle.codeBlockColor = Qt.binding(function() { return panel.darkSurface ? "#8db9f2" : "#305b9c" })
            targetStyle.thematicBreakColor = Qt.binding(function() { return panel.darkSurface ? "#d8a0e5" : "#743a86" })
            targetStyle.thematicBreakThickness = 3
            targetStyle.inlineCodeFont.family = "serif"
            targetStyle.quoteRuleColor = Qt.binding(function() { return panel.darkSurface ? "#d8a0e5" : "#743a86" })
            targetStyle.quoteRuleThickness = 3
            targetStyle.quoteIndent = 24
            targetStyle.listIndent = 32
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
            model: ["Body", "H1", "H2", "H3", "H4", "H5", "H6", "Inline code", "Code blocks"]
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
        Label { text: "Size" }
        ComboBox {
            objectName: "unitSelector"
            model: ["px", "pt"]
            currentIndex: panel.pointUnit ? 1 : 0
            onActivated: panel.changeUnit(currentIndex === 1)
        }
        SpinBox {
            id: sizeField
            objectName: "sizeField"
            readonly property int factor: panel.pointUnit ? 100 : 1
            from: factor; to: 512 * factor; editable: true
            stepSize: factor
            value: Math.round(panel.fontSize * factor)
            textFromValue: function(value, locale) {
                return Number(value / factor).toLocaleString(locale, 'f', panel.pointUnit ? 2 : 0)
            }
            valueFromText: function(text, locale) {
                return Math.round(Number.fromLocaleString(locale, text) * factor)
            }
            validator: SizeValidator {
                locale: sizeField.locale.name
                notation: SizeValidator.StandardNotation
                bottom: 1; top: 512
                decimals: panel.pointUnit ? 2 : 0
            }
            onValueModified: panel.editSize(value / factor)
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
    RowLayout {
        Label { text: "Thematic break color" }
        TextField {
            id: ruleColorField
            objectName: "ruleColorField"
            Layout.preferredWidth: 180
            text: { panel.revision; return panel.targetStyle.thematicBreakColor.toString() }
            readonly property bool validColor: panel.isValidColor(text)
            color: validColor ? palette.text : "#b02020"
            onEditingFinished: panel.applyRuleColor(text)
        }
        Label { text: ruleColorField.validColor ? "" : "Invalid color"; color: "#b02020" }
        Label { text: "Thickness (px)" }
        SpinBox {
            objectName: "ruleThicknessField"
            from: 0; to: 16; editable: true
            value: panel.targetStyle.thematicBreakThickness
            onValueModified: panel.targetStyle.thematicBreakThickness = value
        }
        Item { Layout.fillWidth: true }
    }

    RowLayout {
        Label { text: "Link color" }
        TextField {
            objectName: "linkColorField"; Layout.preferredWidth: 180
            text: { panel.revision; return panel.targetStyle.linkColor.toString() }
            color: panel.isValidColor(text) ? palette.text : "#b02020"
            onEditingFinished: {
                if (panel.isValidColor(text)) { panel.targetStyle.linkColor = text; panel.refresh() }
            }
        }
        CheckBox {
            objectName: "linkUnderlineField"; text: "Underline links"
            checked: panel.targetStyle.linkUnderline
            onToggled: panel.targetStyle.linkUnderline = checked
        }
        Item { Layout.fillWidth: true }
    }
    RowLayout {
        Label { text: "List indent" }
        SpinBox {
            objectName: "listIndentField"; from: 0; to: 96; editable: true
            value: panel.targetStyle.listIndent
            onValueModified: panel.targetStyle.listIndent = value
        }
        Label { text: "Quote indent" }
        SpinBox {
            objectName: "quoteIndentField"; from: 0; to: 96; editable: true
            value: panel.targetStyle.quoteIndent
            onValueModified: panel.targetStyle.quoteIndent = value
        }
        Label { text: "Quote rule" }
        TextField {
            objectName: "quoteColorField"; Layout.preferredWidth: 120
            text: { panel.revision; return panel.targetStyle.quoteRuleColor.toString() }
            color: panel.isValidColor(text) ? palette.text : "#b02020"
            onEditingFinished: panel.applyQuoteColor(text)
        }
        SpinBox {
            objectName: "quoteThicknessField"; from: 0; to: 16; editable: true
            value: panel.targetStyle.quoteRuleThickness
            onValueModified: panel.targetStyle.quoteRuleThickness = value
        }
        Item { Layout.fillWidth: true }
    }

}
