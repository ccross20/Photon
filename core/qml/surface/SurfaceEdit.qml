pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

Rectangle {
    id: root
    color: "#1e1e1e"

    property var selectedGizmo: null
    property bool snapEnabled: true
    property int gridSize: 10

    // Designer toolbar
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

            Button { text: "+ Slider"; onClicked: surfaceView.addGizmo("Slider") }
            Button { text: "+ Toggle"; onClicked: surfaceView.addGizmo("Toggle") }
            Button { text: "+ Palette"; onClicked: surfaceView.addGizmo("Palette") }

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

        // Alignment grid
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

        // Click empty canvas to deselect (below the gizmo frames)
        MouseArea {
            anchors.fill: parent
            onClicked: root.selectedGizmo = null
        }

        Repeater {
            model: surfaceView.gizmos
            delegate: GizmoFrame {
                required property var modelData
                gizmo: modelData
                editMode: true
                selected: root.selectedGizmo === modelData
                snapEnabled: root.snapEnabled
                gridSize: root.gridSize
                onSelectRequested: root.selectedGizmo = modelData
                onRemoveRequested: {
                    if (root.selectedGizmo === modelData)
                        root.selectedGizmo = null;
                    surfaceView.removeGizmo(modelData);
                }
            }
        }
    }
}
