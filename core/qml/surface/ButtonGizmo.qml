import QtQuick

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

    // Pressed/active state is deliberately kept out of the generic property
    // model (see ButtonGizmo.h) so it doesn't show up as an editable field in
    // the Properties panel, which means it has no NOTIFY-backed property of
    // its own for a binding to track - isActive() alone would only ever be
    // read once. valuesChanged() is re-emitted on every setActive() though
    // (see ButtonGizmo::setActive), so resync from it explicitly instead.
    property bool active: gizmo ? gizmo.isActive() : false
    Connections {
        target: root.gizmo
        function onValuesChanged() { root.active = root.gizmo.isActive(); }
    }

    readonly property bool sticky: root.gizmo ? !!root.gizmo.values.isSticky : false
    readonly property color fillColor: root.gizmo
        ? (root.active ? root.gizmo.values.onColor : root.gizmo.values.offColor)
        : "#cccccc"

    Rectangle {
        anchors.fill: parent
        radius: 4
        color: root.fillColor
        border.color: Qt.darker(root.fillColor, 1.3)
        border.width: 1
    }

    Text {
        anchors.fill: parent
        text: root.gizmo && root.gizmo.values.text !== undefined ? root.gizmo.values.text : ""
        color: root.textColorFor(root.fillColor)
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    // A plain MouseArea rather than QtQuick.Controls' Button: Button only
    // reports clicked()/toggled() on release, and is unconditionally a
    // latching toggle - neither matches a console button, which engages the
    // instant you press it (sticky or momentary alike) and, when momentary,
    // releases the moment you let go, wherever the pointer ends up (not just
    // when released back over the button, which is all onClicked covers).
    MouseArea {
        anchors.fill: parent
        onPressed: {
            if (!root.gizmo)
                return;
            root.gizmo.setActive(root.sticky ? !root.gizmo.isActive() : true);
        }
        onReleased: {
            // Sticky already latched on press; only momentary lets go here.
            if (root.gizmo && !root.sticky)
                root.gizmo.setActive(false);
        }
    }
}
