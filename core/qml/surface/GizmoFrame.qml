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
    // Perform-mode interaction hooks (value tooltip), threaded down to leaves
    // and containers alike so it reaches any depth in the tree; null in edit
    // mode, where leaf controls are disabled anyway.
    property var performContext: null
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
            if (t === "Button") return "ButtonGizmo.qml";
            if (t === "Toggle") return "ToggleGizmo.qml";
            if (t === "Dial") return "DialGizmo.qml";
            if (t === "XYPad") return "XYPadGizmo.qml";
            if (t === "Hue") return "HueGizmo.qml";
            if (t === "Segmented") return "SegmentedGizmo.qml";
            if (t === "Palette") return "PaletteGizmo.qml";
            if (t === "Container") return "ContainerGizmo.qml";
            return "";
        }
        onLoaded: {
            content.item.gizmo = frame.gizmo;
            // Not every gizmo type declares performContext (only the ones that
            // show a value tooltip, plus Container to keep threading it down);
            // feature-detect rather than assigning unconditionally.
            if ("performContext" in content.item)
                content.item.performContext = frame.performContext;
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
        // Kept fully inside the frame: a handle straddling the edge would have
        // its outer half fall outside the frame's bounds, where clicks are not
        // delivered to it (they hit the container behind instead).
        x: frame.width - width
        y: frame.height - height
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
        x: frame.width - width
        y: 0
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

    // Drag grip (top-left) — reparent/reorder. Distinct from the body drag so
    // repositioning in an Absolute container and moving between containers stay
    // separate gestures. Drives the SurfaceEdit drag proxy via editContext.
    Rectangle {
        width: 18
        height: 18
        radius: 3
        color: "#3daee9"
        visible: frame.editMode && frame.selected
        x: 0
        y: 0
        z: 3

        Text {
            anchors.centerIn: parent
            text: "✥"
            color: "white"
            font.pixelSize: 11
        }

        DragHandler {
            target: null
            onActiveChanged: {
                if (!frame.editContext)
                    return;
                if (active)
                    frame.editContext.beginDrag(frame.gizmo, centroid.scenePosition);
                else
                    frame.editContext.endDrag();
            }
            onCentroidChanged: if (active && frame.editContext) frame.editContext.moveDrag(centroid.scenePosition)
        }
    }

    // Rotation handle (stalk above top-center), absolute frames only.
    Item {
        visible: frame.editMode && frame.selected && !frame.layoutManaged
        width: 16
        height: 28
        x: frame.width / 2 - width / 2
        y: -height
        z: 3

        Rectangle {
            width: 2
            height: 16
            color: "#2ecc71"
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.bottom: parent.bottom
        }
        Rectangle {
            width: 14
            height: 14
            radius: 7
            color: "#2ecc71"
            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter
        }

        MouseArea {
            anchors.fill: parent
            onPositionChanged: (mouse) => {
                var p = mapToItem(frame.parent, mouse.x, mouse.y);
                var cx = frame.x + frame.width / 2;
                var cy = frame.y + frame.height / 2;
                var deg = Math.atan2(p.y - cy, p.x - cx) * 180 / Math.PI + 90;
                if (frame.editContext && frame.editContext.snapEnabled)
                    deg = Math.round(deg / 15) * 15;
                else
                    deg = Math.round(deg);
                frame.gizmo.setPropValue("rotation", deg);
            }
        }
    }
}
