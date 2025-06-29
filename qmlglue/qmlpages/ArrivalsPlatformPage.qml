import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore
import "../"

Column {
    id : page
    property var stationName
    property var arrivalsInfo
    width: parent.width

    Repeater{
        model: arrivalsInfo
        Column {
            width: parent.width

            RowLayout {
                width: parent.width
                Label {
                    id: timeDue
                    text: new Date(modelData["expectedArrival"]).toLocaleTimeString(undefined, {
                                                                                        hour: '2-digit',
                                                                                        minute: '2-digit',
                                                                                        second: '2-digit'
                                                                                    })
                    font.pixelSize: 20
                    font.bold: true
                    Layout.column: 0
                    Layout.row: index
                    wrapMode: Text.WordWrap
                }

                Column {
                    Layout.preferredWidth: parent.width-2.5*timeDue.width
                    Layout.column: 1
                    Layout.row: index
                    Rectangle {
                        id: rec
                        property var c : cppGlue.getTFLLine(modelData["lineId"])
                        color: c.Color
                        width: parent.width
                        height: myCheckBox.height

                        Label {
                            id: myCheckBox
                            text: rec.c.name
                            color: "white"
                            font.pointSize: 16
                            width: parent.width
                            horizontalAlignment: Qt.AlignHCenter
                        }
                        border.width: 1
                    }

                    Label {
                        width: parent.width
                        text: "[" + modelData["vehicleId"] + "]"
                        visible: modelData["vehicleId"].length > 0
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Qt.AlignLeft
                        font.pixelSize: 12
                    }

                    Label {
                        width: parent.width
                        text: modelData["finalDestination"]
                        wrapMode: Text.WordWrap
                        horizontalAlignment: Qt.AlignLeft
                        font.pixelSize: 18
                    }
                }

                Column {
                    Layout.column: 2
                    Layout.row: index

                    Label {
                        text: modelData["timeToStation"]
                        font.pixelSize: 20
                        wrapMode: Text.WordWrap
                    }

                    Image {
                        id: name
                        source: "qrc:/images/downArrow.png"

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                var params = [modelData["lineId"], modelData["vehicleId"], stationName]
                                stackView.pushPage("qmlpages/ArrivalVehicleStopPoints.qml", params)
                            }
                        }
                    }
                }
            }

            Rectangle {
                width: parent.width
                border.width: 2
                height: 2
            }

            Item {
                height: 2
                width: parent.width
            }
        }
    }
}
