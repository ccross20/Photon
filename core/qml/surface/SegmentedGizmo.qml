pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root
    property var gizmo
    implicitHeight: 56

    readonly property bool hasLabel: root.gizmo && root.gizmo.values.text !== undefined && root.gizmo.values.text.length > 0
    readonly property bool vertical: root.gizmo && root.gizmo.values.orientation === "Vertical"
    readonly property int selectedIndex: root.gizmo ? root.gizmo.values.selectedIndex : -1

    // Options are stored as a single comma-separated string; split/trim it
    // here into the list of segment labels.
    readonly property var optionList: {
        var list = [];
        if (!root.gizmo || !root.gizmo.values.options)
            return list;
        var parts = root.gizmo.values.options.split(",");
        for (var i = 0; i < parts.length; i++) {
            var t = parts[i].trim();
            if (t.length > 0)
                list.push(t);
        }
        return list;
    }

    Text {
        id: label
        visible: root.hasLabel
        text: root.gizmo ? root.gizmo.values.text : ""
        color: "#dddddd"
        font.pixelSize: 13
        anchors.top: parent.top
        anchors.left: parent.left
    }

    // Fills whatever's left of the frame below the label (or the whole frame
    // if there's no label), so the segments scale with the gizmo's box.
    Item {
        id: segmentArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.top: root.hasLabel ? label.bottom : parent.top
        anchors.topMargin: root.hasLabel ? 4 : 0

        readonly property int segCount: Math.max(1, root.optionList.length)
        readonly property real gap: 1
        readonly property real segW: root.vertical ? width : (width - (segCount - 1) * gap) / segCount
        readonly property real segH: root.vertical ? (height - (segCount - 1) * gap) / segCount : height

        Repeater {
            model: root.optionList
            delegate: Rectangle {
                id: segment
                required property string modelData
                required property int index
                width: segmentArea.segW
                height: segmentArea.segH
                x: root.vertical ? 0 : index * (segmentArea.segW + segmentArea.gap)
                y: root.vertical ? index * (segmentArea.segH + segmentArea.gap) : 0
                color: index === root.selectedIndex ? "#3daee9" : "#333333"
                border.color: "#555555"
                border.width: 1

                Text {
                    anchors.centerIn: parent
                    width: parent.width - 8
                    text: segment.modelData
                    color: segment.index === root.selectedIndex ? "#ffffff" : "#cccccc"
                    font.pixelSize: 12
                    horizontalAlignment: Text.AlignHCenter
                    elide: Text.ElideRight
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: if (root.gizmo) root.gizmo.setPropValue("selectedIndex", segment.index)
                }
            }
        }
    }
}
