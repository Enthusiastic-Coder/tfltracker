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

                Component.onCompleted: {
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

            ArrivalsNationalPage {id:arr; qmlArgs: page.qmlArgs}
//            LinesStatusPage {
//                mode: "bus,tube,dlr,overground,tram,elizabeth"
//                lines: cppGlue.getLineIDsforStopId(obj.stationId)
//            }
//            StopPointDisruptionPage {qmlArgs: page.qmlArgs}
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        Settings {
            id: settings
            category: "qml/arrivalNationalStatusPage"
            property alias currentIndex : swipe.currentIndex

            Component.onCompleted: {
                if( currentIndex >= swipe.count)
                    tab.currentIndex = swipe.count-1
            }
        }

        Repeater {
            id:rep
            model: ["Arrivals"/*, "Status", "Disruption"*/]

            TabButton {
                id: control
                text : modelData
                width: Math.max(120, tab.width / rep.model.length)
            }
        }
    }
}
