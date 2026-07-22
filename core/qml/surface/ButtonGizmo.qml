import QtQuick
import QtQuick.Controls

Item {
    id: root
    property var gizmo
    implicitHeight: 56

    // Simple perceived-luminance check so the label stays readable against
    // whatever on/off color the user picks.
    function textColorFor(bg) {
        if (!bg)
            return "#000000";
        var luminance = 0.299 * bg.r + 0.587 * bg.g + 0.114 * bg.b;
        return luminance > 0.5 ? "#000000" : "#ffffff";
    }

    Button {
        id: btn
        anchors.fill: parent
        text: root.gizmo && root.gizmo.values.text !== undefined ? root.gizmo.values.text : ""
        checkable: true
        checked: root.gizmo ? root.gizmo.isActive() : false
        onToggled: if (root.gizmo) root.gizmo.setActive(checked)

        readonly property color fillColor: root.gizmo
            ? (btn.checked ? root.gizmo.values.onColor : root.gizmo.values.offColor)
            : "#cccccc"

        background: Rectangle {
            radius: 4
            color: btn.fillColor
            border.color: Qt.darker(btn.fillColor, 1.3)
            border.width: 1
        }

        contentItem: Text {
            text: btn.text
            color: root.textColorFor(btn.fillColor)
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }
}
