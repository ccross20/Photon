pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#1e1e1e"

    // Edit context threaded through the gizmo tree (see GizmoFrame/ContainerGizmo).
    property var selectedGizmo: null
    property bool snapEnabled: true
    property int gridSize: 10

    // Drag-to-reparent state, driven by GizmoFrame grips and consumed by the
    // drag proxy + container DropAreas.
    property bool dragActive: false
    property var dragGizmo: null
    property point dragScenePos: Qt.point(0, 0)

    function beginDrag(g, scenePos) {
        root.dragGizmo = g;
        root.dragScenePos = scenePos;
        root.dragActive = true;
        dragProxy.Drag.start();
    }
    function moveDrag(scenePos) { root.dragScenePos = scenePos; }
    function endDrag() {
        // Explicit drop() reliably delivers to the DropArea under the hotspot;
        // toggling Drag.active via a binding does not.
        dragProxy.Drag.drop();
        root.dragActive = false;
    }

    function selectGizmo(g) { root.selectedGizmo = g; }
    function removeGizmo(g) {
        if (root.selectedGizmo === g)
            root.selectedGizmo = null;
        surfaceView.removeGizmo(g);
    }

    Shortcut {
        sequences: [StandardKey.Delete, StandardKey.Backspace]
        onActivated: if (root.selectedGizmo) root.removeGizmo(root.selectedGizmo)
    }
    // Add into the selected container, else the root container.
    function addTarget() {
        return (root.selectedGizmo && root.selectedGizmo.gizmoType === "Container")
            ? root.selectedGizmo : surfaceView.root;
    }

    Rectangle {
        id: toolbar
        anchors { top: parent.top; left: parent.left; right: parent.right }
        height: 44
        color: "#2b2b2b"
        z: 10

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 8
            spacing: 6

            Button { text: "+ Slider";    onClicked: surfaceView.addGizmo("Slider", root.addTarget()) }
            Button { text: "+ Button";    onClicked: surfaceView.addGizmo("Button", root.addTarget()) }
            Button { text: "+ Toggle";    onClicked: surfaceView.addGizmo("Toggle", root.addTarget()) }
            Button { text: "+ Palette";   onClicked: surfaceView.addGizmo("Palette", root.addTarget()) }
            Button { text: "+ Container"; onClicked: surfaceView.addGizmo("Container", root.addTarget()) }

            CheckBox {
                text: "Snap"
                checked: root.snapEnabled
                onToggled: root.snapEnabled = checked
            }
        }
    }

    Inspector {
        id: inspector
        anchors { top: toolbar.bottom; right: parent.right; bottom: parent.bottom }
        width: 236
        gizmo: root.selectedGizmo
    }

    Item {
        id: canvasArea
        anchors { top: toolbar.bottom; left: parent.left; right: inspector.left; bottom: parent.bottom }
        clip: true

        Canvas {
            id: grid
            anchors.fill: parent
            onPaint: {
                var ctx = getContext("2d");
                ctx.clearRect(0, 0, width, height);
                ctx.strokeStyle = "#2a2a2a";
                ctx.lineWidth = 1;
                var g = root.gridSize;
                for (var gx = 0; gx < width; gx += g) {
                    ctx.beginPath(); ctx.moveTo(gx, 0); ctx.lineTo(gx, height); ctx.stroke();
                }
                for (var gy = 0; gy < height; gy += g) {
                    ctx.beginPath(); ctx.moveTo(0, gy); ctx.lineTo(width, gy); ctx.stroke();
                }
            }
            onWidthChanged: requestPaint()
            onHeightChanged: requestPaint()
        }

        // Click empty canvas selects the root container itself — the same rule
        // GizmoFrame applies for nested containers (empty space selects the
        // container, not nothing), so the root's own layout/stretch/padding are
        // reachable in the Inspector.
        MouseArea {
            anchors.fill: parent
            onClicked: root.selectGizmo(surfaceView.root)
        }

        ContainerGizmo {
            anchors.fill: parent
            gizmo: surfaceView.root
            editMode: true
            editContext: root
        }

        // Selection outline for the root container. Other gizmos get this via
        // GizmoFrame, but the root has no frame — it can't be moved, resized, or
        // deleted, only selected to edit its layout properties.
        Rectangle {
            anchors.fill: parent
            color: "transparent"
            border.color: root.selectedGizmo === surfaceView.root ? "#3daee9" : "#30ffffff"
            border.width: root.selectedGizmo === surfaceView.root ? 2 : 1
            z: 999
        }
    }

    // Floating proxy that follows the pointer during a reparent drag. Its Drag
    // hotspot is what container DropAreas test against.
    Item {
        id: dragProxy
        z: 1000
        width: 44
        height: 26
        visible: root.dragActive
        x: root.dragScenePos.x - width / 2
        y: root.dragScenePos.y - height / 2

        property var payload: root.dragGizmo

        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        Rectangle {
            anchors.fill: parent
            radius: 4
            color: "#803daee9"
            border.color: "#3daee9"
            border.width: 1
        }
    }
}
