pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls

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

    // Insertion index for a drop at (px,py) in this container's coordinates.
    // Absolute/Tabs append; layouts insert by comparing against child centers.
    function dropIndex(px, py) {
        if (!cont.gizmo)
            return 0;
        var n = cont.gizmo.childItems.length;
        var lt = cont.layoutType;
        if (lt === "Absolute" || lt === "Tabs")
            return n;
        var pos = layoutLoader.item;
        if (!pos)
            return n;
        var lx = px - cont.pad;
        var ly = py - cont.pad;
        var idx = 0;
        for (var i = 0; i < pos.children.length; i++) {
            var c = pos.children[i];
            if (c.gizmo === undefined || c.gizmo === null)
                continue; // skip the Repeater and non-frame children
            if (lt === "Vertical") {
                if (ly > c.y + c.height / 2) idx++;
            } else if (lt === "Horizontal") {
                if (lx > c.x + c.width / 2) idx++;
            } else { // Grid: reading order (row-major)
                var below = ly >= c.y + c.height;
                var sameRow = ly >= c.y && ly < c.y + c.height;
                if (below || (sameRow && lx > c.x + c.width / 2)) idx++;
            }
        }
        return idx;
    }

    // Subtle backing so containers are visible while editing.
    Rectangle {
        anchors.fill: parent
        visible: cont.editMode
        color: "#0affffff"
        border.color: "#20ffffff"
        border.width: 1
    }

    // Reparent/reorder drop target. Behind the content so nested containers'
    // DropAreas take priority for the deepest container under the pointer.
    DropArea {
        id: dropArea
        anchors.fill: parent
        enabled: cont.editMode
        onDropped: (drop) => {
            var g = cont.editContext ? cont.editContext.dragGizmo : null;
            if (!g)
                return;
            // In an Absolute container, drop where the pointer released.
            if (cont.layoutType === "Absolute") {
                var nx = drop.x - cont.pad - g.values.width / 2;
                var ny = drop.y - cont.pad - g.values.height / 2;
                if (cont.editContext && cont.editContext.snapEnabled) {
                    var gs = cont.editContext.gridSize;
                    nx = Math.round(nx / gs) * gs;
                    ny = Math.round(ny / gs) * gs;
                }
                g.setPropValue("x", nx);
                g.setPropValue("y", ny);
            }
            surfaceView.reparentGizmo(g, cont.gizmo, cont.dropIndex(drop.x, drop.y));
        }
        Rectangle {
            anchors.fill: parent
            color: "#203daee9"
            visible: dropArea.containsDrag
        }
    }

    Loader {
        id: layoutLoader
        anchors.fill: parent
        anchors.margins: cont.pad
        sourceComponent: {
            switch (cont.layoutType) {
            case "Vertical":   return colComp;
            case "Horizontal": return rowComp;
            case "Grid":       return gridComp;
            case "Tabs":       return tabsComp;
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

    // ---- Tabs / Pages -------------------------------------------------------
    Component {
        id: tabsComp
        Item {
            anchors.fill: parent

            TabBar {
                id: bar
                width: parent.width
                currentIndex: cont.gizmo ? cont.gizmo.currentPage : 0
                onCurrentIndexChanged: if (cont.gizmo) cont.gizmo.currentPage = bar.currentIndex

                Repeater {
                    model: cont.gizmo.childItems
                    delegate: TabButton {
                        required property var modelData
                        required property int index
                        text: (modelData.values.title && modelData.values.title.length > 0)
                              ? modelData.values.title : ("Page " + (index + 1))
                        width: Math.max(80, implicitWidth)
                        // Selecting a tab also selects that page in edit mode, so
                        // added controls target the page rather than the container.
                        onClicked: if (cont.editMode && cont.editContext) cont.editContext.selectGizmo(modelData)
                    }
                }
            }

            // Add a page (edit only). New pages are Absolute containers, made
            // current and selected so the next added control lands inside them.
            Button {
                visible: cont.editMode
                anchors.left: bar.right
                anchors.leftMargin: 4
                anchors.verticalCenter: bar.verticalCenter
                text: "+"
                width: 32
                onClicked: {
                    surfaceView.addGizmo("Container", cont.gizmo);
                    if (cont.gizmo) {
                        var last = cont.gizmo.childItems.length - 1;
                        cont.gizmo.currentPage = last;
                        if (cont.editContext)
                            cont.editContext.selectGizmo(cont.gizmo.childItems[last]);
                    }
                }
            }

            // Page content: each page fills this area; only the current is shown.
            // Explicit fill (rather than StackLayout) guarantees the page frame
            // has size, so it is selectable by clicking its content.
            Item {
                anchors { top: bar.bottom; left: parent.left; right: parent.right; bottom: parent.bottom }
                Repeater {
                    model: cont.gizmo.childItems
                    delegate: GizmoFrame {
                        required property var modelData
                        required property int index
                        anchors.fill: parent
                        visible: index === (cont.gizmo ? cont.gizmo.currentPage : 0)
                        gizmo: modelData
                        editMode: cont.editMode
                        editContext: cont.editContext
                        layoutManaged: true
                    }
                }
            }
        }
    }
}
