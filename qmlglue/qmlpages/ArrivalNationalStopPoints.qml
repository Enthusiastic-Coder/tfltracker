import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore

ScrollablePage {
    id : page
    property var qmlArgs

    ColumnLayout {

        QtObject {
            id: obj
            property var model
            property var stationId : qmlArgs[0]
            property var stationName : qmlArgs[1]
            property var trainService : qmlArgs[2]

            Component.onCompleted:  {
                lblProgress.text = "Calling Points :" +obj.stationName
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
                    model : obj.trainService["subsequentCallingPoints"]

                    ColumnLayout {
                        width: parent.width

                        RowLayout {
                            Layout.fillWidth: true

                            Column {
                                Layout.preferredWidth: parent.width/4

                                Label {
                                    text : modelData["st"]
                                    wrapMode: Text.Wrap
                                    horizontalAlignment: Text.AlignLeft
                                    width: parent.width

                                    font {
                                        bold:true
                                        pixelSize: 16
                                    }
                                }

                                Label {
                                    text : modelData["et"]
                                    wrapMode: Text.Wrap
                                    horizontalAlignment: Text.AlignLeft
                                    width: parent.width

                                    font {
                                        pixelSize: 14
                                    }
                                }
                            }


                            Label {
                                id: lbl
                                text : modelData["locationName"] + " " + modelData["via"]
                                wrapMode: Text.Wrap
                                horizontalAlignment: Text.AlignLeft
                                Layout.fillWidth: true

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

