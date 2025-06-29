import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore

ScrollablePage {
    id : page
    property var qmlArgs

    function incomingData(model) {

        lblProgress.text = "(updated - " + new Date().toLocaleTimeString().replace("/.*(\d{2}:\d{2}:\d{2}).*/", "$1") + ")";
        obj.model = model
        obj.updateResults()
    }

    Component.onCompleted: {
        cppGlue.onStopPointDisruptionUpdated.connect(incomingData)
        obj.refresh()
    }

    Component.onDestruction: {
        cppGlue.onStopPointDisruptionUpdated.disconnect(incomingData)
    }

    ColumnLayout {

        QtObject {
            id: obj
            property var model
            property var stationId : qmlArgs[0]
            property var stationName : qmlArgs[1]

            function refresh() {
                lblProgress.text = "refreshing please wait...."

                cppGlue.updateDisruptionForStopPoint(obj.stationId)
            }

            function updateResults() {
                loader.sourceComponent = null

                var iError = model["InternetError"];

                if( iError  != null) {
                    loader.sourceComponent = noResults
                    loader.item.msg = iError

                } else if( obj.model["results"].length === 0) {
                    loader.sourceComponent = noResults
                    loader.item.msg = "-- no data --"
                }
                else
                    loader.sourceComponent = results
            }
        }

        Component {
            id: noResults

            Column {
                property string msg
                width: parent.width
                Label {
                    text: msg
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    wrapMode: Text.Wrap
                    font.pixelSize: 30
                    font.bold: true
                }
            }
        }

        Component {
            id: results

            ColumnLayout {
                Layout.fillWidth: true

                Repeater {
                    model : obj.model["results"]

                    Column {
                        Layout.fillWidth: true

                        Text {
                            width: parent.width
                            text: modelData["commonName"]
                            wrapMode: Text.Wrap
                            font.pixelSize: 18
                        }
                        Text {
                            width: parent.width
                            text: modelData["description"].replace(/\\n/g, "\r\n")
                            wrapMode: Text.Wrap
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

        Timer {
            interval: 60000;
            running: true;
            repeat: true
            onTriggered: obj.refresh()
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
    }
}

