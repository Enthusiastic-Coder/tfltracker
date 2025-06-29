import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    property var actions

    id: col

    Repeater {
        model: actions

        RadioDelegate {
            leftPadding: 20
            rightPadding: 20
            text:modelData.text
            Layout.fillWidth: true

            checked: modelData.isChecked
            objectName: modelData.objectName
            onToggled: ui.trigger(objectName)
        }
    }

    //Push items above
    Item {
        implicitHeight: 20
        Layout.fillHeight: true
    }
}
