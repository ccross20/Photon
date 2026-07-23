pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root
    property var gizmo
    // Set (only in Perform mode) so this gizmo can show a floating value
    // tooltip near the cursor while being dragged; see SurfacePerform.qml.
    property var performContext: null
    implicitHeight: 64

    readonly property bool vertical: root.gizmo && root.gizmo.values.orientation === "Vertical"
    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0

    readonly property real fraction: {
        if (!root.gizmo)
            return 0;
        var min = root.gizmo.values.min;
        var max = root.gizmo.values.max;
        return (max > min) ? Math.max(0, Math.min(1, (root.gizmo.values.value - min) / (max - min))) : 0;
    }

    Text {
        id: label
        visible: root.hasLabel
        text: root.gizmo ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
        anchors.top: parent.top
        anchors.left: root.vertical ? undefined : parent.left
        anchors.horizontalCenter: root.vertical ? parent.horizontalCenter : undefined
    }

    // Fills whatever's left of the frame below the label (or the whole frame
    // if there's no label) — both the touch area and the visual fill cover
    // the entire gizmo box, rather than a thin QtQuick Slider groove.
    Item {
        id: track
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        // Vertical: up = max, matching the Toggle/Dial/XY Pad convention.
        function updateFromPoint(px, py) {
            if (!root.gizmo)
                return;
            var t = root.vertical
                ? 1 - Math.max(0, Math.min(1, py / track.height))
                : Math.max(0, Math.min(1, px / track.width));
            var min = root.gizmo.values.min;
            var max = root.gizmo.values.max;
            root.gizmo.setPropValue("value", min + t * (max - min));
        }

        // Unfilled side.
        Rectangle {
            anchors.fill: parent
            radius: 4
            color: root.gizmo ? root.gizmo.values.trackColor : "#333333"
            border.color: "#555555"
            border.width: 1
        }

        // Filled side, up to the current value. Grows rightward from the
        // left edge (horizontal) or upward from the bottom edge (vertical).
        Rectangle {
            radius: 4
            color: root.gizmo ? root.gizmo.values.fillColor : "#3daee9"
            anchors.left: parent.left
            anchors.right: root.vertical ? parent.right : undefined
            anchors.top: root.vertical ? undefined : parent.top
            anchors.bottom: parent.bottom
            width: root.vertical ? undefined : parent.width * root.fraction
            height: root.vertical ? parent.height * root.fraction : undefined
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            onPressed: (mouse) => {
                track.updateFromPoint(mouse.x, mouse.y);
                if (root.performContext && root.gizmo)
                    root.performContext.showValueTooltip(root.gizmo.values.value.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onPositionChanged: (mouse) => {
                track.updateFromPoint(mouse.x, mouse.y);
                if (root.performContext && root.gizmo)
                    root.performContext.moveValueTooltip(root.gizmo.values.value.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onReleased: if (root.performContext) root.performContext.hideValueTooltip()
        }
    }
}
