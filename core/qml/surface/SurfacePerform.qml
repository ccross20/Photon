pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root
    color: "#1e1e1e"

    // Free-form canvas: each gizmo is positioned by its own layout properties
    // (x/y/width/height/rotation) rather than stacked in a column.
    Item {
        id: canvas
        anchors.fill: parent

        Repeater {
            model: surfaceView.gizmos
            delegate: Loader {
                id: gizmoLoader
                required property var modelData

                x: gizmoLoader.modelData.values.x
                y: gizmoLoader.modelData.values.y
                width: gizmoLoader.modelData.values.width
                height: gizmoLoader.modelData.values.height
                rotation: gizmoLoader.modelData.values.rotation

                source: {
                    var t = gizmoLoader.modelData.gizmoType;
                    if (t === "Slider") return "SliderGizmo.qml";
                    if (t === "Toggle") return "ToggleGizmo.qml";
                    if (t === "Palette") return "PaletteGizmo.qml";
                    return "";
                }
                onLoaded: item.gizmo = gizmoLoader.modelData
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
