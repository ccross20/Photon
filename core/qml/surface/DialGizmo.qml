import QtQuick

Item {
    id: root
    property var gizmo
    // Set (only in Perform mode) so this gizmo can show a floating value
    // tooltip near the cursor while being dragged; see SurfacePerform.qml.
    property var performContext: null
    implicitHeight: 96

    readonly property bool continuous: root.gizmo && root.gizmo.values.continuous
    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0

    function mod(n, m) { return ((n % m) + m) % m; }

    // Current pointer angle in degrees, 0 = up, clockwise positive. Bounded
    // dials sweep -135..135 mapped from [min,max]; a continuous dial stores
    // its accumulated rotation directly in `value` and just wraps it for
    // display.
    readonly property real pointerDeg: {
        if (!root.gizmo)
            return -135;
        if (root.continuous)
            return root.mod(root.gizmo.values.value, 360);
        var min = root.gizmo.values.min;
        var max = root.gizmo.values.max;
        var t = (max > min) ? (root.gizmo.values.value - min) / (max - min) : 0;
        t = Math.max(0, Math.min(1, t));
        return -135 + t * 270;
    }

    Text {
        id: label
        visible: root.hasLabel
        text: root.gizmo ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    // Fills whatever's left of the frame below the label (or the whole frame
    // if there's no label), so the dial scales with the gizmo's box.
    Item {
        id: dialArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        readonly property real diameter: Math.max(20, Math.min(width, height))

        function pointerAngle(x, y) {
            var cx = dialArea.width / 2;
            var cy = dialArea.height / 2;
            var deg = Math.atan2(y - cy, x - cx) * 180 / Math.PI + 90;
            if (deg > 180) deg -= 360;
            if (deg < -180) deg += 360;
            return deg;
        }

        Rectangle {
            id: body
            anchors.centerIn: parent
            width: dialArea.diameter
            height: dialArea.diameter
            radius: width / 2
            color: "#333333"
            border.color: "#555555"
            border.width: 1
        }

        Item {
            id: knob
            anchors.centerIn: body
            width: body.width
            height: body.height
            rotation: root.pointerDeg

            Rectangle {
                width: 3
                height: parent.height / 2 - 6
                radius: 1.5
                color: "#3daee9"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.top: parent.top
                anchors.topMargin: 6
            }
        }

        MouseArea {
            id: dragArea
            anchors.fill: parent
            property real lastAngle: 0

            onPressed: (mouse) => {
                dragArea.lastAngle = dialArea.pointerAngle(mouse.x, mouse.y);
                if (root.performContext && root.gizmo)
                    root.performContext.showValueTooltip(root.gizmo.values.value.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onPositionChanged: (mouse) => {
                if (!root.gizmo)
                    return;
                var angle = dialArea.pointerAngle(mouse.x, mouse.y);
                if (root.continuous) {
                    var delta = angle - dragArea.lastAngle;
                    if (delta > 180) delta -= 360;
                    if (delta < -180) delta += 360;
                    dragArea.lastAngle = angle;
                    root.gizmo.setPropValue("value", root.gizmo.values.value + delta);
                } else {
                    var clamped = Math.max(-135, Math.min(135, angle));
                    var t = (clamped + 135) / 270;
                    var min = root.gizmo.values.min;
                    var max = root.gizmo.values.max;
                    root.gizmo.setPropValue("value", min + t * (max - min));
                }
                if (root.performContext)
                    root.performContext.moveValueTooltip(root.gizmo.values.value.toFixed(2),
                        dragArea.mapToItem(root.performContext, mouse.x, mouse.y));
            }
            onReleased: if (root.performContext) root.performContext.hideValueTooltip()
        }
    }
}
