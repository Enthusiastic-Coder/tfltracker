import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore

ScrollablePage {
    id : page
    property var qmlArgs

    function incomingData(model) {

        lblProgress.text = model["lineName"] + " >> " +model["destinationName"]

        lblProgress2.text = "Vehicle Id: [" + model["vehicleId"] + "]<br>(updated - " + new Date().toLocaleTimeString().replace("/.*(\d{2}:\d{2}:\d{2}).*/", "$1") + ")";

        obj.model = model
        obj.updateResults()
    }

    Component.onCompleted: {
        cppGlue.onVehicleArrivalInfoUpdated.connect(incomingData)
        obj.refresh()
    }

    Component.onDestruction: {
        cppGlue.onVehicleArrivalInfoUpdated.disconnect(incomingData)
    }


    ColumnLayout {

        QtObject {
            id: obj
            property var model
            property var lineId : qmlArgs[0]
            property var vehicleId : qmlArgs[1]
            property var stationName: qmlArgs[2]

            function refresh() {
                lblProgress.text = "refreshing please wait...."

                cppGlue.updateVehicleArrivalInfo(obj.lineId, obj.vehicleId, obj.stationName)
            }

            function updateResults() {
                loader.sourceComponent = null

                var iError = model["InternetError"];

                if( iError  !== undefined) {
                    loader.sourceComponent = noResults
                    loader.item.msg = iError

                } else
                    loader.sourceComponent = results
            }
        }

        Component {
            id: noResults

            Column {
                property string msg
                Layout.fillWidth: true
                Label {
                    width: parent.width
                    text: msg
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.Wrap
                    font.pixelSize: 30
                    font.bold: true
                }
            }
        }

        Component {
            id: results

            ColumnLayout {
                width: parent.width

                Repeater {
                    model : obj.model["stopPoints"]

                    Column {
                       Layout.fillWidth: true

                        RowLayout {
                            width: parent.width

                            ColumnLayout {
                                Layout.fillWidth: true

                                Label {
                                    text : modelData["expectedArrival"]
                                    wrapMode: Text.Wrap
                                    width: parent.width

                                    font {
                                        bold:true
                                        pixelSize: 16
                                    }
                                }

                                Label {
                                    text : modelData["timeToStation"]
                                    wrapMode: Text.Wrap
                                    width: parent.width

                                    font {
                                        pixelSize: 14
                                    }
                                }
                            }

                            Item {
                                Layout.preferredWidth: 20
                                height :1
                            }

                            Label {
                                id: lbl
                                text : modelData["stationName"]
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignLeft
                                Layout.fillWidth: true

                                font {
                                    pixelSize: 18
                                    bold : modelData["stationName"] === obj.stationName
                                }
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 2
                            border.width: 1
                        }
                    }
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 4
            border.width: 2
        }

        RowLayout {

            Label {
                id: lblProgress
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                font {
                    bold:true
                    pixelSize: 20
                }
            }
        }

        RowLayout {

            Label {
                id: lblProgress2
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
        }

        Rectangle {
            Layout.fillWidth: true
            height: 4
            border.width: 1
        }

        Loader {
            id: loader
            Layout.fillWidth: true
        }

        Rectangle {
            height: 4
            border.width: 2
            border.color: "Black"
            Layout.fillWidth: true
        }

        Timer {
            interval: 15000;
            running: true;
            repeat: true
            onTriggered: obj.refresh()
        }
    }
}

