import QtQuick

Item {
    id: root
    property var gizmo
    implicitHeight: 140

    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0
    readonly property real hue: root.gizmo ? root.gizmo.values.hue : 0

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
    // if there's no label), so the wheel scales with the gizmo's box.
    Item {
        id: wheelArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        readonly property real diameter: Math.max(20, Math.min(width, height))
        readonly property real outerR: diameter / 2 - 2
        readonly property real innerR: outerR * 0.6
        readonly property real midR: (outerR + innerR) / 2

        // Angle is measured the same way as the Dial gizmo: 0 = up, clockwise
        // positive, wrapped into [0,360) so it maps directly onto hue.
        function updateFromPoint(px, py) {
            if (!root.gizmo)
                return;
            var cx = wheelArea.width / 2;
            var cy = wheelArea.height / 2;
            var dx = px - cx;
            var dy = py - cy;
            if (Math.sqrt(dx * dx + dy * dy) > wheelArea.outerR + 8)
                return;
            var deg = Math.atan2(dy, dx) * 180 / Math.PI + 90;
            deg = ((deg % 360) + 360) % 360;
            root.gizmo.setPropValue("hue", deg);
        }

        Canvas {
            id: wheel
            anchors.centerIn: parent
            width: wheelArea.diameter
            height: wheelArea.diameter

            // Paints the ring as 360 one-degree wedges of Qt.hsva(), i.e. a
            // full-saturation, full-value rainbow — the classic hue wheel.
            // Canvas angles are 0 = right/clockwise, but hue (and the handle
            // below) use 0 = up/clockwise, so each wedge is shifted -90deg to
            // keep the painted color under the handle at the same hue.
            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                var cx = width / 2;
                var cy = height / 2;
                var mid = (wheelArea.outerR + wheelArea.innerR) / 2;
                var thickness = wheelArea.outerR - wheelArea.innerR;
                for (var i = 0; i < 360; i++) {
                    ctx.strokeStyle = Qt.hsva(i / 360, 1, 1, 1);
                    ctx.lineWidth = thickness + 1;
                    ctx.beginPath();
                    ctx.arc(cx, cy, mid, (i - 90 - 1) * Math.PI / 180, (i - 90 + 1) * Math.PI / 180);
                    ctx.stroke();
                }
            }
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
        }

        // Center swatch previewing the fully-saturated selected hue.
        Rectangle {
            width: Math.max(4, wheelArea.innerR * 2 - 8)
            height: width
            radius: width / 2
            anchors.centerIn: wheel
            color: Qt.hsva(root.hue / 360, 1, 1, 1)
            border.color: "#40000000"
            border.width: 1
        }

        // Handle riding the ring at the selected hue.
        Rectangle {
            id: handle
            width: 14
            height: 14
            radius: width / 2
            color: Qt.hsva(root.hue / 360, 1, 1, 1)
            border.color: "#ffffff"
            border.width: 2
            x: wheel.x + wheel.width / 2 + Math.sin(root.hue * Math.PI / 180) * wheelArea.midR - width / 2
            y: wheel.y + wheel.height / 2 - Math.cos(root.hue * Math.PI / 180) * wheelArea.midR - height / 2
        }

        MouseArea {
            anchors.fill: parent
            onPressed: (mouse) => wheelArea.updateFromPoint(mouse.x, mouse.y)
            onPositionChanged: (mouse) => wheelArea.updateFromPoint(mouse.x, mouse.y)
        }
    }
}
