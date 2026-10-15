pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown
import QMarkdown.Private

Item {
    id: imageRow
    required property MarkdownStyle style
    required property ViewState coordinator
    required property color searchHighlightColor
    required property color currentSearchHighlightColor
    property int textAlignment: Qt.AlignLeft
    signal linkActivated(string destination)

    required property var imageData
    required property string imageLink
    required property int imageLinkId
    required property bool imageLinked
    required property int selectionOffset
    required property int selectionLength
    height: imagePaint.implicitHeight
    ImageItem {
        id: imagePaint
        objectName: "markdownImage"
        property int projectionOffset: imageRow.selectionOffset
        property int projectionLength: imageRow.selectionLength
        property bool imageLinked: imageRow.imageLinked
        property string imageLink: imageRow.imageLink
        SelectionRegistration {
            coordinator: imageRow.coordinator
            item: imagePaint
            kind: ViewState.ImageSelection
            offset: imageRow.selectionOffset
            length: imageRow.selectionLength
            imageLinkId: imageRow.imageLinkId
            imageLink: imageRow.imageLink
            imageLinked: imageRow.imageLinked
        }
        Rectangle {
            anchors.fill: parent
            z: 1
            color: imageRow.style.selectionBackgroundColor
            visible: imageRow.coordinator.selectionEnabled
                && imageRow.coordinator.selectionStart < imageRow.coordinator.selectionEnd
                && imageRow.selectionLength > 0
                && imageRow.coordinator.selectionStart < imageRow.selectionOffset + imageRow.selectionLength
                && imageRow.coordinator.selectionEnd > imageRow.selectionOffset
        }
        Rectangle {
            anchors.fill: parent
            property var matches: imageRow.coordinator.searchRevision > 0
                ? imageRow.coordinator.searchRangesFor(imageRow.selectionOffset, imageRow.selectionLength) : []
            visible: matches.length > 0
            color: matches.some(function (match) {
                return match.current
            }) ? imageRow.currentSearchHighlightColor : imageRow.searchHighlightColor
        }
        Rectangle {
            objectName: "markdownImageFocus"
            anchors.fill: parent
            z: 2
            color: "transparent"
            border.width: 2
            border.color: imageRow.coordinator.linkController.color
            visible: imageRow.imageLinkId >= 0
                && imageRow.imageLinkId === imageRow.coordinator.linkController.focusedId
        }
        image: imageRow.imageData
        width: Math.min(imageRow.width, imagePaint.naturalWidth)
        height: implicitHeight
        x: imageRow.textAlignment === Qt.AlignRight ? imageRow.width - width
            : imageRow.textAlignment === Qt.AlignHCenter ? (imageRow.width - width) / 2
            : 0
        HoverHandler {
            cursorShape: imageRow.imageLinked ? Qt.PointingHandCursor : Qt.ArrowCursor
        }
        TapHandler {
            acceptedButtons: Qt.LeftButton
            gesturePolicy: TapHandler.DragThreshold
            enabled: imageRow.imageLinked
            acceptedDevices: imageRow.coordinator.selectionEnabled ? PointerDevice.TouchScreen | PointerDevice.Stylus
                : PointerDevice.AllDevices
            onTapped: function (eventPoint, button) {
                if (eventPoint.position.x >= 0
                    && eventPoint.position.x < imagePaint.width
                    && eventPoint.position.y >= 0
                    && eventPoint.position.y < imagePaint.height)
                    imageRow.linkActivated(imageRow.imageLink)
            }
        }
    }
}
