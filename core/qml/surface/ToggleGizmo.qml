import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var gizmo
    implicitHeight: 56

    Button {
        anchors.fill: parent
        text: root.gizmo && root.gizmo.values.text !== undefined ? root.gizmo.values.text : ""
        checkable: true
        checked: root.gizmo ? root.gizmo.isActive() : false
        onToggled: if (root.gizmo) root.gizmo.setActive(checked)
    }
}
