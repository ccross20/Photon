pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root
    color: "#1e1e1e"

    // Value tooltip shown while dragging a slider/dial/XY pad, driven by
    // gizmos through `performContext` (threaded down via ContainerGizmo /
    // GizmoFrame). Positions are already in this item's coordinate space —
    // gizmos map their local pointer position here via mapToItem so the
    // tooltip renders correctly regardless of nesting depth.
    property bool tooltipVisible: false
    property string tooltipText: ""
    property point tooltipPos: Qt.point(0, 0)

    function showValueTooltip(text, scenePos) {
        root.tooltipText = text;
        root.tooltipPos = scenePos;
        root.tooltipVisible = true;
    }
    function moveValueTooltip(text, scenePos) {
        root.tooltipText = text;
        root.tooltipPos = scenePos;
    }
    function hideValueTooltip() {
        root.tooltipVisible = false;
    }

    ContainerGizmo {
        anchors.fill: parent
        gizmo: surfaceView.root
        editMode: false
        editContext: null
        performContext: root
    }

    Text {
        anchors.centerIn: parent
        visible: surfaceView.root ? surfaceView.root.childItems.length === 0 : true
        text: "No gizmos yet. Switch to Edit mode to add controls."
        color: "#888888"
        font.pixelSize: 13
    }

    // Floating value readout. Offset above and clear of the cursor/finger so
    // it stays readable rather than sitting directly under the touch point.
    Rectangle {
        id: tooltip
        visible: root.tooltipVisible
        z: 1000
        radius: 8
        color: "#ee1a1a1a"
        border.color: "#666666"
        border.width: 2
        width: tooltipLabel.implicitWidth + 32
        height: tooltipLabel.implicitHeight + 20
        x: root.tooltipPos.x - width / 2
        y: root.tooltipPos.y - height - 40

        Text {
            id: tooltipLabel
            anchors.centerIn: parent
            text: root.tooltipText
            color: "#ffffff"
            font.pixelSize: 24
            font.bold: true
        }
    }
}
