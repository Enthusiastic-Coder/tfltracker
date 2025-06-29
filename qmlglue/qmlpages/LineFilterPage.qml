import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../"
import CppClassLib

ColumnLayout {
    property var menu
    property bool isBoat : false
    width: parent.width
    id: col

    QtObject {
        id: d
        property var actions : menu.actions
    }

    Component.onCompleted: {

        if( isBoat)
            loader.sourceComponent = riveBusComponent

        else
            loader.sourceComponent = tubeComponent

    }

    Column {
        Layout.fillWidth: true
        spacing: 10

        Loader {
            width: parent.width
            id: loader
        }
    }

    Component {
        id:tubeComponent
        GridLayout {
            columns: 1
            width: parent.width

            QtObject {
                id: obj
                property int count :0
            }

            Repeater {
                model:menu

                Repeater {
                    model: modelData.actions

                    Switch {
                        id: controlTube
                        text:modelData.data.name
                        checked: modelData.data.Visible

                        contentItem: Text {
                            text: controlTube.text
                            font: controlTube.font
                            opacity: enabled ? 1.0 : 0.3
                            color: "white"
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: controlTube.indicator.width + controlTube.spacing
                        }

                        onToggled: {

                            if( checked && fetchListTooBig())
                                checked = false

                            modelData.data.Visible = checked
                        }

                        Layout.column: 0
                        Layout.row: obj.count++
                        Layout.preferredWidth: parent.width*0.75
                        Layout.alignment: Qt.AlignHCenter
                        Rectangle {
                            anchors.fill: parent
                            color: modelData.data.Color
                        }
                    }
                }
            }
        }
    }

    Component {
        id:riveBusComponent

        GridLayout {
            id: gridLayout
            columns: 2
            width: parent.width

            QtObject {
                id: obj
                property int count :0
                property int count2 : 0
            }

            Repeater {
                model:menu

                Repeater {
                    model: modelData.actions

                    Switch {
                        id: control
                        text:modelData.data.name
                        checked: modelData.data.Visible

                        contentItem: Text {
                            text: control.text
                            font: control.font
                            opacity: enabled ? 1.0 : 0.3
                            color: color
                            verticalAlignment: Text.AlignVCenter
                            leftPadding: control.indicator.width + control.spacing
                        }

                        onToggled: {

                            if( checked && fetchListTooBig())
                                checked = false

                            modelData.data.Visible = checked
                        }

                        Layout.row: obj.count++
                        Layout.column: 0
                        Layout.alignment: Qt.AlignLeft
                    }
                }
            }
            Repeater {
                model:menu

                Repeater {
                    model: modelData.actions

                    ColorPickerRectangle {
                        id: colorPicker
                        color: modelData.data.Color
                        Layout.preferredHeight: 40
                        Layout.alignment: Qt.AlignLeft
                        border.width: 1
                        enabled: !modelData.data.ReadOnly
                        Layout.row: obj.count2++
                        Layout.column: 1
                        onColorChanged: modelData.data.Color = color
                    }
                }
            }
        }
    }

    //Push items above
    Item {
        implicitHeight: 20
        Layout.fillHeight: true
    }
}
