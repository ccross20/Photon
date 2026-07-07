pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

// Property inspector for the selected gizmo. Renders one editor per property,
// driven entirely by the gizmo's propertyDefs() so no gizmo-specific UI is
// needed. Values read from the reactive `values` map and write via setPropValue.
Rectangle {
    id: inspector

    property var gizmo
    // Recomputed whenever the selected gizmo changes.
    property var allDefs: inspector.gizmo ? inspector.gizmo.propertyDefs() : []

    color: "#252525"

    ColorDialog {
        id: colorDialog
        property string targetId
        onAccepted: if (inspector.gizmo) inspector.gizmo.setPropValue(colorDialog.targetId, colorDialog.selectedColor)
    }

    // ---- editor components -------------------------------------------------

    Component {
        id: numberEditor
        TextField {
            property var def
            property real modelValue: inspector.gizmo ? Number(inspector.gizmo.values[def.id]) : 0
            Component.onCompleted: text = modelValue
            onModelValueChanged: if (!activeFocus) text = modelValue
            onEditingFinished: if (inspector.gizmo) inspector.gizmo.setPropValue(def.id, parseFloat(text))
            validator: DoubleValidator {}
            selectByMouse: true
        }
    }

    Component {
        id: textEditor
        TextField {
            property var def
            property string modelValue: inspector.gizmo ? String(inspector.gizmo.values[def.id]) : ""
            Component.onCompleted: text = modelValue
            onModelValueChanged: if (!activeFocus) text = modelValue
            onEditingFinished: if (inspector.gizmo) inspector.gizmo.setPropValue(def.id, text)
            selectByMouse: true
        }
    }

    Component {
        id: boolEditor
        Switch {
            property var def
            checked: inspector.gizmo ? inspector.gizmo.values[def.id] : false
            onToggled: if (inspector.gizmo) inspector.gizmo.setPropValue(def.id, checked)
        }
    }

    Component {
        id: colorEditor
        Rectangle {
            id: swatch
            property var def
            implicitHeight: 26
            radius: 3
            color: inspector.gizmo ? inspector.gizmo.values[swatch.def.id] : "black"
            border.color: "#555"
            border.width: 1
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    colorDialog.targetId = swatch.def.id;
                    colorDialog.selectedColor = inspector.gizmo.values[swatch.def.id];
                    colorDialog.open();
                }
            }
        }
    }

    Component {
        id: optionsEditor
        ComboBox {
            property var def
            model: (def.metadata && def.metadata.options) ? def.metadata.options : []
            Component.onCompleted: currentIndex = find(inspector.gizmo ? inspector.gizmo.values[def.id] : "")
            onActivated: if (inspector.gizmo) inspector.gizmo.setPropValue(def.id, currentText)
        }
    }

    function editorFor(type) {
        switch (type) {
        case "Boolean": return boolEditor;
        case "Text":    return textEditor;
        case "Color":   return colorEditor;
        case "Options": return optionsEditor;
        default:        return numberEditor; // Number (and Point fields fall back)
        }
    }

    // ---- one labelled row per property -------------------------------------

    component PropertyRow: RowLayout {
        required property var def
        Layout.fillWidth: true
        spacing: 6
        Text {
            text: parent.def.name
            color: "#bbb"
            font.pixelSize: 12
            Layout.preferredWidth: 64
        }
        Loader {
            Layout.fillWidth: true
            sourceComponent: inspector.editorFor(parent.def.type)
            onLoaded: item.def = parent.def
        }
    }

    // ---- panel -------------------------------------------------------------

    ScrollView {
        anchors.fill: parent
        anchors.margins: 8
        contentWidth: availableWidth

        ColumnLayout {
            width: inspector.width - 16
            spacing: 6

            Text {
                text: inspector.gizmo ? inspector.gizmo.gizmoType : "No selection"
                color: "#ddd"
                font.bold: true
                font.pixelSize: 13
            }

            Text {
                visible: inspector.gizmo
                text: "Layout"
                color: "#888"
                font.pixelSize: 11
                Layout.topMargin: 6
            }
            Repeater {
                model: inspector.allDefs.filter(function(d) { return d.category === "layout"; })
                delegate: PropertyRow { required property var modelData; def: modelData }
            }

            Text {
                visible: inspector.gizmo
                text: "Properties"
                color: "#888"
                font.pixelSize: 11
                Layout.topMargin: 6
            }
            Repeater {
                model: inspector.allDefs.filter(function(d) { return d.category !== "layout"; })
                delegate: PropertyRow { required property var modelData; def: modelData }
            }
        }
    }
}
