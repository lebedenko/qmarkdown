pragma ComponentBehavior: Bound
import QtQuick
import QMarkdown.Private

FormattedText {
    id: selectable
    required property ViewState coordinator
    required property color highlightColor
    required property color currentHighlightColor
    property int projectionOffset: 0
    property int projectionLength: 0
    // Qt 6.8 may elide an unused revision read; make it part of the result.
    searchRanges: coordinator.searchRevision > 0
        ? coordinator.searchRangesFor(projectionOffset, projectionLength) : []
    searchHighlightColor: highlightColor
    currentSearchHighlightColor: currentHighlightColor
    selectionStart: coordinator.selectionEnabled ? Math.max(0, coordinator.selectionStart - projectionOffset) : 0
    selectionEnd: coordinator.selectionEnabled ? Math.max(0, coordinator.selectionEnd - projectionOffset) : 0
    implicitHeight: logicalHeight
    SelectionRegistration {
        coordinator: selectable.coordinator
        item: selectable
        offset: selectable.projectionOffset
        length: selectable.projectionLength
    }
}
