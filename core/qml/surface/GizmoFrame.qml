pragma ComponentBehavior: Bound

import QtQuick

// Wraps one gizmo (leaf control or nested container). The parent container's
// positioner sets this frame's geometry; the frame does not bind its own x/y/
// width/height, which keeps absolute drags from fighting layout positioning.
// Edit state (selection, snap) is threaded down via `editContext` so selection
// works at any depth in the tree.
Item {
    id: frame

    property var gizmo
    property bool editMode: false
    property var editContext: null
    // True when a layout container owns this frame's position (Vertical/
    // Horizontal/Grid) — move-drag is disabled, only resize/select/delete apply.
    property bool layoutManaged: false

    readonly property bool isContainer: frame.gizmo && frame.gizmo.gizmoType === "Container"
    readonly property bool selected: frame.editContext && frame.editContext.selectedGizmo === frame.gizmo

    function snap(v) {
        return (frame.editContext && frame.editContext.snapEnabled)
            ? Math.round(v / frame.editContext.gridSize) * frame.editContext.gridSize : v;
    }

    Loader {
        id: content
        anchors.fill: parent
        // Leaf controls are disabled while editing so drags hit the frame;
        // containers stay enabled so their children remain selectable.
        enabled: frame.isContainer || !frame.editMode
        source: {
            var t = frame.gizmo ? frame.gizmo.gizmoType : "";
            if (t === "Slider") return "SliderGizmo.qml";
            if (t === "Toggle") return "ToggleGizmo.qml";
            if (t === "Palette") return "PaletteGizmo.qml";
            if (t === "Container") return "ContainerGizmo.qml";
            return "";
        }
        onLoaded: {
            content.item.gizmo = frame.gizmo;
            if (frame.isContainer) {
                content.item.editMode = frame.editMode;
                content.item.editContext = frame.editContext;
            }
        }
    }

    // Selection outline (visual only)
    Rectangle {
        anchors.fill: parent
        visible: frame.editMode
        color: "transparent"
        border.color: frame.selected ? "#3daee9" : "#30ffffff"
        border.width: frame.selected ? 2 : 1
        z: 2
    }

    // Select + move. For containers this sits behind the content (z -1) so
    // child frames get clicks first and only empty container space selects the
    // container; for leaves it sits on top.
    MouseArea {
        anchors.fill: parent
        visible: frame.editMode
        z: frame.isContainer ? -1 : 1
        property real startX
        property real startY
        property point startPos

        onPressed: (mouse) => {
            if (frame.editContext)
                frame.editContext.selectGizmo(frame.gizmo);
            if (!frame.layoutManaged) {
                startX = frame.gizmo.values.x;
                startY = frame.gizmo.values.y;
                startPos = mapToItem(frame.parent, mouse.x, mouse.y);
            }
        }
        onPositionChanged: (mouse) => {
            if (frame.layoutManaged) return;
            var p = mapToItem(frame.parent, mouse.x, mouse.y);
            frame.gizmo.setPropValue("x", frame.snap(startX + (p.x - startPos.x)));
            frame.gizmo.setPropValue("y", frame.snap(startY + (p.y - startPos.y)));
        }
    }

    // Resize handle (bottom-right) — absolute frames only
    Rectangle {
        width: 16
        height: 16
        radius: 3
        color: "#3daee9"
        visible: frame.editMode && frame.selected && !frame.layoutManaged
        x: frame.width - width / 2
        y: frame.height - height / 2
        z: 3

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

    // Delete button (top-right)
    Rectangle {
        width: 18
        height: 18
        radius: 9
        color: "#c0392b"
        visible: frame.editMode && frame.selected
        x: frame.width - width / 2
        y: -height / 2
        z: 3

        Text {
            anchors.centerIn: parent
            text: "×"
            color: "white"
            font.pixelSize: 12
        }

        MouseArea {
            anchors.fill: parent
            onClicked: if (frame.editContext) frame.editContext.removeGizmo(frame.gizmo)
        }
    }
}
