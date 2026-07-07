pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root
    color: "#1e1e1e"

    ContainerGizmo {
        anchors.fill: parent
        gizmo: surfaceView.root
        editMode: false
        editContext: null
    }

    Text {
        anchors.centerIn: parent
        visible: surfaceView.root ? surfaceView.root.childItems.length === 0 : true
        text: "No gizmos yet. Switch to Edit mode to add controls."
        color: "#888888"
        font.pixelSize: 13
    }
}
