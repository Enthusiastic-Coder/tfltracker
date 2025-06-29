import QtQuick
import QtQuick.Controls

Item {
    property var actions
    implicitHeight: col.height
    property real widthFactor: 1.0

    Column {
        id: col
        width: parent.width * widthFactor
        spacing: 20
        anchors.horizontalCenter: parent.horizontalCenter

        Repeater {
            model: actions
            Button {
                id: but
                text: modelData.text

                contentItem: Label {
                    text: but.text
                    font: but.font
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                }

                objectName: modelData.objectName
                visible: modelData.isVisible
                enabled: !modelData.isDisabled
                onClicked: ui.trigger(objectName)
                width: parent.width
            }
        }
    }
}
