import QtQuick
import QtQuick.Controls

Column {
    id : col
    property var actions

    Repeater {
        model: actions
        SwitchDelegate {
            text: modelData.text
            objectName: modelData.objectName
            width: parent.width
            visible: modelData.isVisible
            enabled: !modelData.isDisabled
            checked: modelData.isChecked
            onToggled: ui.toggle(objectName, checked)
        }
    }
}
