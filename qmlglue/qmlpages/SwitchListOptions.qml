import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id : col
    property var actions

    Repeater {
        model: actions
        Switch {
            text: modelData.text
            objectName: modelData.objectName
            Layout.fillWidth: true
            visible: modelData.isVisible
            enabled: !modelData.isDisabled
            checked: modelData.isChecked
            onToggled: ui.toggle(objectName, checked)
        }
    }
}
