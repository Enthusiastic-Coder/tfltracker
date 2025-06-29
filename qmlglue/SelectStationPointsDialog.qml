import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib

FlickableDialog {
    title: "Multiple Items found\nSelect:"
    property var qmlArgs
    width: parent.width *9/10
    closePolicy: Popup.CloseOnEscape| Popup.CloseOnPressOutside

    Column {

        Rectangle {
            width: parent.width
            height: 4
            border.width: 2
            border.color: "black"
        }

        Repeater {
            model: qmlArgs

            Column {

                QtObject {
                    id: obj
                    property int isStopPoint : modelData["type"]
                    property var stopId: modelData["id"]
                    property var name : modelData["name"]
                    property var stopLetter: modelData["stopLetter"]
                    property var towards: modelData["towards"]
                    property var mode: modelData["mode"]

                    function getIconFilename() {
                        return  "/images/tfl/" + mode + ".png";
                    }
                }

                width: parent.width

                RowLayout {
                    spacing: 10
                    width: parent.width

                    Image {
                        source: obj.getIconFilename()
                    }
                    RoundButton {
                        id : but
                        text: obj.stopLetter.length === 0  ? "Select" : obj.stopLetter
                        font.pixelSize: 22
                        font.bold: true
                        Layout.fillWidth: true
                        contentItem: Label {
                            text: but.text
                            font: but.font
                            horizontalAlignment: Text.AlignLeft
                        }
                        onClicked: {
                            if( obj.isStopPoint == 1 )
                                radarView.triggerArrivalStatusPage(obj.stopId, obj.name, obj.stopLetter, obj.towards)
                            else
                                if( obj.isStopPoint == 2 )
                                    radarView.triggerArrivalNationalRailStatusPage(obj.stopId, obj.name)
                            else
                                radarView.selectVehicle(obj.stopId)

                            accept()
                        }
                    }
                }

                RowLayout {
                    spacing: 10
                    width: parent.width
                    Label {
                        font.pixelSize: 18
                        text: obj.name
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                    }
                }

                RowLayout {
                    spacing: 10
                    width: parent.width
                    visible: obj.towards != null && obj.towards.length > 0
                    Label {
                        font.pixelSize: 14
                        text: "(towards "+ obj.towards +")"
                        Layout.fillWidth: true
                        horizontalAlignment: Text.AlignLeft
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 2
                    border.width: 2
                    border.color: "black"
                }
            }
        }
    }
}
