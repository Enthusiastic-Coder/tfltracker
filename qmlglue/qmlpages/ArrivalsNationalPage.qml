import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore

ScrollablePage {
    id : page
    property var qmlArgs

    function incomingData(model) {

        lblProgress.text = "<b>Powered by National Rail Enquiries</b><br> (updated - " + new Date().toLocaleTimeString().replace("/.*(\d{2}:\d{2}:\d{2}).*/", "$1") + ")";
        obj.model = model
        obj.updateResults()
    }

    Component.onCompleted: {
        cppGlue.onNationalRailUpdate.connect(incomingData)
        obj.refresh()
    }

    Component.onDestruction: {
        cppGlue.onNationalRailUpdate.disconnect(incomingData)
    }

    ColumnLayout {

        QtObject {
            id: obj
            property var model
            //            property var platforms : []
            property var stationId : qmlArgs[0]
            property var stationName : qmlArgs[1]

            function refresh() {
                lblProgress.text = "refreshing please wait...."

                cppGlue.updateNationalRailArrivalsForStopPoint(obj.stationId)
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

            Column {
                Layout.fillWidth: true

                Repeater {
                    model : obj.model["trainServices"]

                    ColumnLayout {
                        width: parent.width

                        MouseArea {
                            anchors.fill: parent
                            acceptedButtons: Qt.LeftButton
                            onClicked: {
                                var params = [obj.stationId, obj.stationName, modelData]
                                stackView.pushPage("qmlpages/ArrivalNationalStopPoints.qml", params)
                            }
                        }

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text : modelData["sta"].length !== 0?modelData["sta"] : modelData["std"]
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignLeft

                                font {
                                    bold:true
                                    pixelSize: 20
                                }
                            }

                            Column {
                                Layout.fillWidth: true

                                Label {
                                    text : modelData["toLocationName"] + " " + modelData["via"]
                                    wrapMode: Text.Wrap
                                    horizontalAlignment: Text.AlignLeft
                                    width: parent.width

                                    font {
                                        bold:true
                                        pixelSize: 16
                                    }
                                }

                                Label {
                                    text : "(" + modelData["operator"] + ")"
                                    wrapMode: Text.Wrap
                                    horizontalAlignment: Text.AlignLeft
                                    width: parent.width

                                    font {
                                        pixelSize: 14
                                    }
                                }

                                Label {
                                    text : (modelData["eta"].length !== 0?modelData["eta"] : modelData["etd"]) + " " + (modelData["length"].length != 0 ? "(" + modelData["length"] + " coaches)" : "")
                                    wrapMode: Text.Wrap
                                    width: parent.width
                                    horizontalAlignment: Text.AlignLeft

                                    font {
                                        pixelSize: 16
                                    }
                                }
                            }

                            Image {
                                id: name
                                source: "qrc:/images/downArrow.png"

                            }

                            Label {
                                text :  "Plat : " + modelData["platform"]
                                visible: modelData["platform"].length !== 0
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignRighth/3

                                font {
                                    pixelSize: 18
                                }
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 2
                            border.width: 1
                        }
                    }
                }
            }
        }

        Timer {
            interval: 15000;
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

