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
        cppGlue.onArrivalsUpdated.connect(incomingData)
        obj.refresh()
    }

    Component.onDestruction: {
        cppGlue.onArrivalsUpdated.disconnect(incomingData)
    }

    ColumnLayout {

        Loader {
            id: dynamicLoadDialog
            onStatusChanged: if( item != null) item.open()
            Connections {
                target: dynamicLoadDialog.item
                function onFinished() {
                    obj.lineWanted = dynamicLoadDialog.item.lineWanted
                    obj.refresh()
                    dynamicLoadDialog.sourceComponent = null
                }
            }
        }

        Component {
            id: selectLinesComponent
            SelectLinesDialogPage {
                signal finished()
                onAccepted: finished()
                onRejected: finished()
            }
        }

        QtObject {
            id: obj
            property var model
            property var platforms : []
            property var stationId : qmlArgs[0]
            property var stationName : qmlArgs[1]
            property var busLetter : qmlArgs[2]
            property var towards : qmlArgs[3]
            property var lineWanted : qmlArgs[4]
            property var lines : cppGlue.getLineIDsforStopId(obj.stationId)

            function refresh() {
                lblProgress.text = "refreshing please wait...."

                if( obj.lineWanted === undefined)
                    cppGlue.updateArrivalsForStopPoint(obj.stationId, [])
                else
                    cppGlue.updateArrivalsForStopPoint(obj.stationId, obj.lineWanted)

            }

            function updateResults() {
                loader.sourceComponent = null

                var iError = model["InternetError"];

                if( iError  != null) {
                    loader.sourceComponent = noResults
                    loader.item.msg = iError

                } else {

                    platforms = []

                    var totalArrivals = 0

                    for( var i in obj.model) {

                        totalArrivals += obj.model[i].length
                        platforms.push(i)
                    }

                    platforms.sort( function(a,b) {
                        var pA = parseInt(a.substring(a.lastIndexOf(" ")+1))
                        var pB = parseInt(b.substring(b.lastIndexOf(" ")+1))
                        return pA < pB ? -1 : 1
                    });

                    if( totalArrivals === 0) {
                        loader.sourceComponent = noResults;
                        loader.item.msg = "--- no data ---"
                    } else {

                        if( obj.busLetter.length === 0)
                            loader.sourceComponent = results
                        else
                            loader.sourceComponent = busResults
                    }
                }
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
                Layout.fillWidth: true

                Repeater {
                    model : obj.platforms

                    Column {
                        Layout.fillWidth: true

                        Label {
                            text : modelData
                            wrapMode: Text.Wrap
                            width: parent.width
                            horizontalAlignment: Text.AlignHCenter

                            font {
                                bold:true
                                pixelSize: 20
                            }
                        }

                        Rectangle {
                            width: parent.width
                            height: 2
                            border.width: 1
                        }

                        ArrivalsPlatformPage {
                            width: parent.width
                            arrivalsInfo: obj.model[modelData]
                            stationName: obj.stationName
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

        Component {
            id: busResults

            Column {
                Layout.fillWidth: true

                Repeater {
                    model : obj.platforms

                    ArrivalsPlatformPage {
                        width: parent.width
                        arrivalsInfo: obj.model[modelData]
                        stationName: obj.stationName
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

            RoundButton {
                id: linesButton
                text: "Lines"
                onClicked: {
                    dynamicLoadDialog.sourceComponent = selectLinesComponent
                    dynamicLoadDialog.item.lines = obj.lines
                    dynamicLoadDialog.item.lineWanted = obj.lineWanted===undefined ? [] : obj.lineWanted
                }
            }

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

