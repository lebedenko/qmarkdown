pragma ComponentBehavior: Bound
import QtQuick
import "private/Layout.js" as Layout

Rectangle {
    required property var headingStyle
    property bool suppressed: false
    readonly property bool enabledDecoration: !suppressed && headingStyle !== null && headingStyle.separatorVisible
    readonly property real decorationSpacing: enabledDecoration ? Layout.metric(headingStyle.separatorSpacing) : 0
    height: enabledDecoration ? Layout.metric(headingStyle.separatorThickness) : 0
    color: headingStyle !== null ? headingStyle.separatorColor : "transparent"
    objectName: "headingSeparator"
}
