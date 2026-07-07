pragma ComponentBehavior: Bound

import QtQuick

// Renders a container gizmo by arranging its children according to its `layout`
// property. Each child is a GizmoFrame, which recurses into ContainerGizmo when
// the child is itself a container. Absolute positions children by their x/y;
// Vertical/Horizontal/Grid arrange them (children are layoutManaged). When
// `stretch` is set, layout children share the container's extent equally.
Item {
    id: cont

    property var gizmo
    property bool editMode: false
    property var editContext: null

    readonly property string layoutType: cont.gizmo ? (cont.gizmo.values.layout || "Absolute") : "Absolute"
    readonly property real pad: cont.gizmo ? (cont.gizmo.values.padding || 0) : 0
    readonly property real gap: cont.gizmo ? (cont.gizmo.values.spacing || 0) : 0
    readonly property int cols: cont.gizmo ? Math.max(1, cont.gizmo.values.columns || 1) : 1
    readonly property bool stretch: cont.gizmo ? (cont.gizmo.values.stretch || false) : false
    readonly property int count: cont.gizmo ? cont.gizmo.childItems.length : 0

    // Subtle backing so containers are visible while editing.
    Rectangle {
        anchors.fill: parent
        visible: cont.editMode
        color: "#0affffff"
        border.color: "#20ffffff"
        border.width: 1
    }

    Loader {
        anchors.fill: parent
        anchors.margins: cont.pad
        sourceComponent: {
            switch (cont.layoutType) {
            case "Vertical":   return colComp;
            case "Horizontal": return rowComp;
            case "Grid":       return gridComp;
            default:           return absComp;
            }
        }
    }

    // ---- Absolute -----------------------------------------------------------
    Component {
        id: absComp
        Item {
            anchors.fill: parent
            Repeater {
                model: cont.gizmo.childItems
                delegate: GizmoFrame {
                    required property var modelData
                    gizmo: modelData
                    editMode: cont.editMode
                    editContext: cont.editContext
                    layoutManaged: false
                    x: modelData.values.x
                    y: modelData.values.y
                    width: modelData.values.width
                    height: modelData.values.height
                    rotation: modelData.values.rotation
                }
            }
        }
    }

    // ---- Vertical -----------------------------------------------------------
    Component {
        id: colComp
        Column {
            id: colPositioner
            width: parent ? parent.width : 0
            spacing: cont.gap
            readonly property real cellH: (parent && cont.count > 0)
                ? Math.max(20, (parent.height - (cont.count - 1) * cont.gap) / cont.count) : 20
            Repeater {
                model: cont.gizmo.childItems
                delegate: GizmoFrame {
                    required property var modelData
                    gizmo: modelData
                    editMode: cont.editMode
                    editContext: cont.editContext
                    layoutManaged: true
                    width: colPositioner.width
                    height: cont.stretch ? colPositioner.cellH : modelData.values.height
                }
            }
        }
    }

    // ---- Horizontal ---------------------------------------------------------
    Component {
        id: rowComp
        Row {
            id: rowPositioner
            height: parent ? parent.height : 0
            spacing: cont.gap
            readonly property real cellW: (parent && cont.count > 0)
                ? Math.max(30, (parent.width - (cont.count - 1) * cont.gap) / cont.count) : 30
            Repeater {
                model: cont.gizmo.childItems
                delegate: GizmoFrame {
                    required property var modelData
                    gizmo: modelData
                    editMode: cont.editMode
                    editContext: cont.editContext
                    layoutManaged: true
                    height: rowPositioner.height
                    width: cont.stretch ? rowPositioner.cellW : modelData.values.width
                }
            }
        }
    }

    // ---- Grid ---------------------------------------------------------------
    Component {
        id: gridComp
        Grid {
            id: gridPositioner
            columns: cont.cols
            spacing: cont.gap
            readonly property int rowCount: Math.max(1, Math.ceil(cont.count / cont.cols))
            readonly property real cellW: parent
                ? Math.max(30, (parent.width - (cont.cols - 1) * cont.gap) / cont.cols) : 30
            readonly property real cellH: parent
                ? Math.max(20, (parent.height - (gridPositioner.rowCount - 1) * cont.gap) / gridPositioner.rowCount) : 20
            Repeater {
                model: cont.gizmo.childItems
                delegate: GizmoFrame {
                    required property var modelData
                    gizmo: modelData
                    editMode: cont.editMode
                    editContext: cont.editContext
                    layoutManaged: true
                    width: cont.stretch ? gridPositioner.cellW : modelData.values.width
                    height: cont.stretch ? gridPositioner.cellH : modelData.values.height
                }
            }
        }
    }
}
