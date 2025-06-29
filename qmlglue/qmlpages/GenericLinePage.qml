import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore
import "../"

ColumnLayout {
    width: parent.width * 0.9
    property var line_group

    Repeater {
        model: line_group.actions

        RowLayout {
            Layout.fillWidth: true

            Switch {
                id: line
                text:modelData.data.name
                checked: modelData.data.Visible

                contentItem: Text {
                    text: line.text
                    font: line.font
                    opacity: enabled ? 1.0 : 0.3
                    verticalAlignment: Text.AlignVCenter
                    leftPadding: line.indicator.width + line.spacing
                }

                onToggled: modelData.data.Visible = checked

                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
            }

            ColorPickerRectangle {
                id: colorPicker
                color: modelData.data.Color
                Layout.preferredHeight: 40
                Layout.alignment: Qt.AlignLeft
                border.width: 1
                enabled: !modelData.data.ReadOnly
                onColorChanged: modelData.data.Color = color
            }
        }
    }
}



