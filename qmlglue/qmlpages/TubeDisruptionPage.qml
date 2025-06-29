import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore
import QtQuick.Layouts

ScrollablePage {
    id : page

    function incomingData(model) {

        lblProgress.text = "(updated - " + new Date().toLocaleTimeString().replace("/.*(\d{2}:\d{2}:\d{2}).*/", "$1") + ")";

        obj.model = model
        obj.updateResults()
    }

    Component.onCompleted: {

        cppGlue.onTubeDisruptionUpdated.connect(incomingData)
        obj.refresh()
    }

    Component.onDestruction: cppGlue.onTubeDisruptionUpdated.disconnect(incomingData)

    ColumnLayout {
        QtObject {
            id: obj
            property var model

            function refresh() {
                lblProgress.text = "refreshing please wait...."
                cppGlue.updateTubeDisruptionResults()
            }

            function updateResults() {
                loader.sourceComponent = null
                loader.sourceComponent = results
            }
        }

        Component {
            id: results

            Column {

                Repeater {
                    model : obj.model

                    Column {
                        width: parent.width
                        Text {
                            width: parent.width
                            text: modelData["commonName"] + " [" + modelData["stationAtcoCode"]+"]"
                            wrapMode: Text.Wrap
                            font.pixelSize: 18
                        }
                        Text {
                            width: parent.width
                            text: modelData["description"]
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
