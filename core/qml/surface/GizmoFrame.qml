pragma ComponentBehavior: Bound

import QtQuick

// Renders one gizmo at its layout geometry. In edit mode it overlays designer
// chrome (selection outline, move, resize, delete). Geometry is written back to
// the model live during a drag, so the x/y/width/height bindings below stay
// intact (no imperative-vs-binding conflict). Drags map the pointer into canvas
// space and apply a delta from the grab point, which is stable even as the frame
// itself moves under the cursor.
Item {
    id: frame

    property var gizmo
    property bool editMode: false
    property bool selected: false
    property bool snapEnabled: true
    property int gridSize: 10

    signal selectRequested()
    signal removeRequested()

    x: frame.gizmo ? frame.gizmo.values.x : 0
    y: frame.gizmo ? frame.gizmo.values.y : 0
    width: frame.gizmo ? frame.gizmo.values.width : 120
    height: frame.gizmo ? frame.gizmo.values.height : 40
    rotation: frame.gizmo ? frame.gizmo.values.rotation : 0

    function snap(v) {
        return frame.snapEnabled ? Math.round(v / frame.gridSize) * frame.gridSize : v;
    }

    Loader {
        id: content
        anchors.fill: parent
        // Disable the underlying control while editing so drags move the box
        // instead of operating the control.
        enabled: !frame.editMode
        source: {
            var t = frame.gizmo ? frame.gizmo.gizmoType : "";
            if (t === "Slider") return "SliderGizmo.qml";
            if (t === "Toggle") return "ToggleGizmo.qml";
            if (t === "Palette") return "PaletteGizmo.qml";
            return "";
        }
        onLoaded: content.item.gizmo = frame.gizmo
    }

    // Selection outline + move
    Rectangle {
        anchors.fill: parent
        visible: frame.editMode
        color: "transparent"
        border.color: frame.selected ? "#3daee9" : "#40ffffff"
        border.width: frame.selected ? 2 : 1

        MouseArea {
            anchors.fill: parent
            property real startX
            property real startY
            property point startPos

            onPressed: (mouse) => {
                frame.selectRequested();
                startX = frame.gizmo.values.x;
                startY = frame.gizmo.values.y;
                startPos = mapToItem(frame.parent, mouse.x, mouse.y);
            }
            onPositionChanged: (mouse) => {
                var p = mapToItem(frame.parent, mouse.x, mouse.y);
                frame.gizmo.setPropValue("x", frame.snap(startX + (p.x - startPos.x)));
                frame.gizmo.setPropValue("y", frame.snap(startY + (p.y - startPos.y)));
            }
        }
    }

    // Resize handle (bottom-right), only when selected
    Rectangle {
        width: 16
        height: 16
        radius: 3
        color: "#3daee9"
        visible: frame.editMode && frame.selected
        x: frame.width - width / 2
        y: frame.height - height / 2

        MouseArea {
            anchors.fill: parent
            property real startW
            property real startH
            property point startPos

            onPressed: (mouse) => {
                startW = frame.gizmo.values.width;
                startH = frame.gizmo.values.height;
                startPos = mapToItem(frame.parent, mouse.x, mouse.y);
            }
            onPositionChanged: (mouse) => {
                var p = mapToItem(frame.parent, mouse.x, mouse.y);
                frame.gizmo.setPropValue("width", frame.snap(Math.max(40, startW + (p.x - startPos.x))));
                frame.gizmo.setPropValue("height", frame.snap(Math.max(30, startH + (p.y - startPos.y))));
            }
        }
    }

    // Delete button (top-right), only when selected
    Rectangle {
        width: 18
        height: 18
        radius: 9
        color: "#c0392b"
        visible: frame.editMode && frame.selected
        x: frame.width - width / 2
        y: -height / 2

        Text {
            anchors.centerIn: parent
            text: "×"
            color: "white"
            font.pixelSize: 12
        }

        MouseArea {
            anchors.fill: parent
            onClicked: frame.removeRequested()
        }
    }
}
