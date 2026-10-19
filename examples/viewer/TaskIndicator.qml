pragma ComponentBehavior: Bound
import QtQuick
import QtQuick.Controls
import QMarkdown

Item {
    id: adapter
    required property bool checked
    required property MarkdownStyle markdownStyle
    property real artworkScale: 1
    property alias hostPalette: control.palette
    implicitWidth: control.indicator ? control.indicator.implicitWidth * artworkScale : 0
    implicitHeight: control.indicator ? control.indicator.implicitHeight * artworkScale : 0
    Accessible.ignored: true
    CheckBox {
        id: control
        objectName: "viewerTaskControl"
        visible: false
        enabled: true
        focusPolicy: Qt.NoFocus
        checked: adapter.checked
        Accessible.ignored: true
    }
    // Keep style-owned anchors in an unscaled coordinate space. Scaling the
    // indicator itself would offset centered artwork inside the larger adapter.
    Item {
        id: artwork
        width: control.indicator ? control.indicator.implicitWidth : 0
        height: control.indicator ? control.indicator.implicitHeight : 0
        scale: adapter.artworkScale
        transformOrigin: Item.TopLeft
    }
    Binding { target: control.indicator; property: "parent"; value: artwork }
    Binding { target: control.indicator; property: "x"; value: 0 }
    Binding { target: control.indicator; property: "y"; value: 0 }
    Binding { target: control.indicator; property: "visible"; value: true }
}
