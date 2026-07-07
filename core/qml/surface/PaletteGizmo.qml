pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root
    property var gizmo
    implicitHeight: 56

    Row {
        anchors.fill: parent
        spacing: 6

        Repeater {
            model: root.gizmo ? root.gizmo.swatchColors() : []
            delegate: Rectangle {
                id: swatch
                required property var modelData
                required property int index
                width: 48
                height: parent.height
                radius: 4
                color: swatch.modelData
                border.color: "#000000"
                border.width: 1

                MouseArea {
                    anchors.fill: parent
                    onClicked: if (root.gizmo) root.gizmo.selectSwatch(swatch.index)
                }
            }
        }
    }
}
