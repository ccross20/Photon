import QtQuick

Item {
    id: root
    property var gizmo
    implicitHeight: 56

    readonly property bool on: root.gizmo ? root.gizmo.values.value : false
    readonly property bool vertical: root.gizmo && root.gizmo.values.orientation === "Vertical"
    readonly property color trackColor: root.gizmo
        ? (root.on ? root.gizmo.values.onColor : root.gizmo.values.offColor)
        : "#cccccc"
    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0

    Text {
        id: label
        visible: root.hasLabel
        text: root.gizmo ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
        anchors.top: parent.top
        anchors.left: root.vertical ? undefined : parent.left
        anchors.horizontalCenter: root.vertical ? parent.horizontalCenter : undefined
    }

    // Fills whatever's left of the frame below the label (or the whole frame
    // if there's no label), so the switch scales with the gizmo's box instead
    // of sitting at a fixed size in the corner.
    Item {
        id: switchArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        // The pill's rounded ends are always capped by its thickness — the
        // dimension perpendicular to the direction the handle slides.
        readonly property real crossSize: root.vertical ? width : height

        Rectangle {
            id: track
            anchors.fill: parent
            radius: switchArea.crossSize / 2
            color: root.trackColor
            border.color: Qt.darker(root.trackColor, 1.3)
            border.width: 1

            Behavior on color { ColorAnimation { duration: 120 } }
        }

        Rectangle {
            id: handle
            width: switchArea.crossSize - 4
            height: switchArea.crossSize - 4
            radius: width / 2
            color: "#ffffff"
            border.color: "#30000000"
            border.width: 1

            // Centered on the cross axis; slides between the two ends of the
            // track along the main axis based on state. Vertical: on = top.
            x: root.vertical ? (track.width - width) / 2 : (root.on ? track.width - width - 2 : 2)
            y: root.vertical ? (root.on ? 2 : track.height - height - 2) : (track.height - height) / 2

            Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
            Behavior on y { NumberAnimation { duration: 120; easing.type: Easing.OutCubic } }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: if (root.gizmo) root.gizmo.setActive(!root.on)
        }
    }
}
