import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var gizmo
    implicitHeight: 64

    Column {
        anchors.fill: parent
        spacing: 4

        Text {
            text: root.gizmo && root.gizmo.values.text !== undefined ? root.gizmo.values.text : ""
            color: "#dddddd"
            font.pixelSize: 13
        }

        Slider {
            width: parent.width
            from: root.gizmo ? root.gizmo.values.min : 0
            to: root.gizmo ? root.gizmo.values.max : 1
            value: root.gizmo ? root.gizmo.values.value : 0
            onMoved: if (root.gizmo) root.gizmo.setPropValue("value", value)
        }
    }
}
