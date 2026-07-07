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

    function selectGizmo(g) { root.selectedGizmo = g; }
    function removeGizmo(g) {
        if (root.selectedGizmo === g)
            root.selectedGizmo = null;
        surfaceView.removeGizmo(g);
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

        // Click empty canvas to deselect (below the container).
        MouseArea {
            anchors.fill: parent
            onClicked: root.selectedGizmo = null
        }

        ContainerGizmo {
            anchors.fill: parent
            gizmo: surfaceView.root
            editMode: true
            editContext: root
        }
    }
}
