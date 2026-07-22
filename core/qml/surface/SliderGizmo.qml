import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var gizmo
    readonly property bool vertical: root.gizmo && root.gizmo.values.orientation === "Vertical"
    implicitHeight: 64

    // Horizontal: label above a horizontal slider, both filling the width.
    Column {
        anchors.fill: parent
        spacing: 4
        visible: !root.vertical

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

    // Vertical: label above a vertical slider that fills the remaining height,
    // both centered — meant for tall/narrow gizmo boxes.
    Text {
        id: vLabel
        visible: root.vertical
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        text: root.gizmo && root.gizmo.values.text !== undefined ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
    }

    Slider {
        visible: root.vertical
        orientation: Qt.Vertical
        anchors.top: vLabel.bottom
        anchors.topMargin: 4
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        from: root.gizmo ? root.gizmo.values.min : 0
        to: root.gizmo ? root.gizmo.values.max : 1
        value: root.gizmo ? root.gizmo.values.value : 0
        onMoved: if (root.gizmo) root.gizmo.setPropValue("value", value)
    }
}
