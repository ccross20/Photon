import QtQuick

Item {
    id: root
    property var gizmo
    // Set (only in Perform mode) so this gizmo can show a floating value
    // tooltip near the cursor while being dragged; see SurfacePerform.qml.
    property var performContext: null
    implicitHeight: 140

    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0

    Text {
        id: label
        visible: root.hasLabel
        text: root.gizmo ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
        anchors.top: parent.top
        anchors.left: parent.left
    }

    // Fills whatever's left of the frame below the label (or the whole frame
    // if there's no label), so the pad scales with the gizmo's box.
    Item {
        id: padArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        readonly property real handleSize: 16

        // Normalized [0,1] handle position from the gizmo's current value
        // mapped through its min/max range. Y is flipped so up = max, matching
        // a joystick/XY-pad convention rather than raw screen coordinates.
        readonly property real nx: {
            if (!root.gizmo)
                return 0.5;
            var min = root.gizmo.values.minX;
            var max = root.gizmo.values.maxX;
            return (max > min) ? Math.max(0, Math.min(1, (root.gizmo.values.valueX - min) / (max - min))) : 0.5;
        }
        readonly property real ny: {
            if (!root.gizmo)
                return 0.5;
            var min = root.gizmo.values.minY;
            var max = root.gizmo.values.maxY;
            return (max > min) ? Math.max(0, Math.min(1, (root.gizmo.values.valueY - min) / (max - min))) : 0.5;
        }

        function updateFromPoint(px, py) {
            if (!root.gizmo)
                return;
            var tx = Math.max(0, Math.min(1, px / padArea.width));
            var ty = 1 - Math.max(0, Math.min(1, py / padArea.height));
            var minX = root.gizmo.values.minX;
            var maxX = root.gizmo.values.maxX;
            var minY = root.gizmo.values.minY;
            var maxY = root.gizmo.values.maxY;
            root.gizmo.setPropValue("valueX", minX + tx * (maxX - minX));
            root.gizmo.setPropValue("valueY", minY + ty * (maxY - minY));
        }

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: "#222222"
            border.color: "#555555"
            border.width: 1
        }

        // Center crosshair guides.
        Rectangle {
            width: parent.width
            height: 1
            color: "#333333"
            anchors.verticalCenter: parent.verticalCenter
        }
        Rectangle {
            width: 1
            height: parent.height
            color: "#333333"
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Rectangle {
            id: handle
            width: padArea.handleSize
            height: padArea.handleSize
            radius: width / 2
            color: "#3daee9"
            border.color: "#ffffff"
            border.width: 1
            x: padArea.nx * (padArea.width - width)
            y: (1 - padArea.ny) * (padArea.height - height)
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            onPressed: (mouse) => {
                padArea.updateFromPoint(mouse.x, mouse.y);
                if (root.performContext && root.gizmo)
                    root.performContext.showValueTooltip(
                        "X: " + root.gizmo.values.valueX.toFixed(2) + "  Y: " + root.gizmo.values.valueY.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onPositionChanged: (mouse) => {
                padArea.updateFromPoint(mouse.x, mouse.y);
                if (root.performContext && root.gizmo)
                    root.performContext.moveValueTooltip(
                        "X: " + root.gizmo.values.valueX.toFixed(2) + "  Y: " + root.gizmo.values.valueY.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onReleased: if (root.performContext) root.performContext.hideValueTooltip()
        }
    }
}
