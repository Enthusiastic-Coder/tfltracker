import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore
import QtQuick.Layouts

ScrollablePage {
    id : page
    property var qmlArgs

    Column {

        Column {
            id: colHdg
            width: parent.width
            QtObject {
                id: obj

                property var model: ui.lineGroup_tube.actions

                property var stationId : qmlArgs[0]
                property var stationName : qmlArgs[1]
                property var busLetter : qmlArgs[2]
                property var towards: qmlArgs[3]
                property var lineNames : []

                Component.onCompleted: {
                    var lines = cppGlue.getLineNamesForStopId(obj.stationId)

                    lineNames="[ "
                    for( var i=0;i < lines.length; ++i) {
                        lineNames += lines[i]
                        if( i < lines.length-1)
                            lineNames += ", "
                        else
                            lineNames += " "
                    }

                    lineNames += "]"
                }
            }

            Label {
                id: stnNameLbl
                text : obj.stationName
                width : parent.width
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter

                font {
                    pixelSize: 16
                    bold: true
                }
            }

            Label {
                id: towardsLbl
                text : "(towards " + obj.towards + ")"
                visible: obj.towards != null && obj.towards.length > 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                width : parent.width
                font {
                    pixelSize: 12
                    bold: false
                }
            }

            Label {
                id: stnId
                text : obj.stationId
                horizontalAlignment: Text.AlignHCenter
                width : parent.width
                wrapMode: Text.Wrap

                font {
                    pixelSize: 16
                    bold: true
                }
            }

            Label {
                id: stopPointLbl
                text : "Bus Stop :" + obj.busLetter + ":"
                visible: obj.busLetter.length > 0
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
                width : parent.width
                font {
                    pixelSize: 16
                    bold: true
                }
            }

            Label {
                text : obj.lineNames
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width : parent.width
                font {
                    pixelSize: 12
                }
            }

            PageIndicator {
                id : pi
                count: tab.count
                currentIndex: swipe.currentIndex
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }

        SwipeView {
            id : swipe
            width: parent.width
            height: Math.max(page.height-tab.height-colHdg.height, currentItem !==null ? currentItem.implicitHeight :0)
            currentIndex: tab.currentIndex

            ArrivalsPage {id:arr; qmlArgs: page.qmlArgs}
            LinesStatusPage {
                mode: "bus,tube,dlr,overground,tram,elizabeth-line"
                lines: cppGlue.getLineIDsforStopId(obj.stationId)
            }
            StopPointDisruptionPage {qmlArgs: page.qmlArgs}
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        // Settings {
        //     id: settings
        //     category: "qml/arrivalStatusPage"
        //     property alias currentIndex : swipe.currentIndex

        //     Component.onCompleted: {
        //         if( currentIndex >= swipe.count)
        //             tab.currentIndex = swipe.count-1
        //     }
        // }

        Repeater {
            id:rep
            model: ["Arrivals", "Status", "Disruption"]

            TabButton {
                id: control
                text : modelData
                width: Math.max(120, tab.width / rep.model.length)
            }
        }
    }
}
