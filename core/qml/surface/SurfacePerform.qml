pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root
    color: "#1e1e1e"

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 12
        contentWidth: width
        contentHeight: col.height
        clip: true

        Column {
            id: col
            width: flick.width
            spacing: 10

            Repeater {
                model: surfaceView.gizmos
                delegate: Loader {
                    id: gizmoLoader
                    required property var modelData
                    width: col.width
                    height: item ? item.implicitHeight : 0
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

            Text {
                visible: surfaceView.gizmos.length === 0
                text: "No gizmos yet. Switch to Edit mode to add controls."
                color: "#888888"
                font.pixelSize: 13
            }
        }
    }
}
