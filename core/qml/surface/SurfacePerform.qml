pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root
    color: "#1e1e1e"

    // Free-form canvas: each gizmo is positioned by its own layout geometry.
    // Perform mode reuses GizmoFrame without the editing chrome, so controls
    // receive input directly.
    Item {
        id: canvas
        anchors.fill: parent

        Repeater {
            model: surfaceView.gizmos
            delegate: GizmoFrame {
                required property var modelData
                gizmo: modelData
                editMode: false
            }
        }
    }

    Text {
        anchors.centerIn: parent
        visible: surfaceView.gizmos.length === 0
        text: "No gizmos yet. Switch to Edit mode to add controls."
        color: "#888888"
        font.pixelSize: 13
    }
}
