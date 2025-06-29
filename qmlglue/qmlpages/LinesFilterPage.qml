import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore

ScrollablePage {
    id : page

    Component.onDestruction: {

        cppGlue.updateFetchFilterList()
    }

    Column {
        width: parent.width
        spacing:25

        Component {
            id: tube
            LineFilterPage {
                menu: [ui.lineGroup_tube,ui.lineGroup_cable_car]
            }
        }

        Component {
            id: bus
            LineFilterBusPage{}
        }

        Component {
            id: riverbus
            LineFilterPage {
                menu: [ui.lineGroup_river_bus,ui.lineGroup_river_tour]
                isBoat: true
            }
        }

        PageIndicator {
            id: pi
            count: tab.count
            currentIndex: swipe.currentIndex
            anchors.horizontalCenter: parent.horizontalCenter
        }

        QtObject {
            id:priv
            property var model: [tube, bus, riverbus]
            property var titles: ["Tube", "Bus" ,"RiverBus"]
        }

        SwipeView {
            id : swipe
            width: parent.width
            height: Math.max(page.height-tab.height-pi.height, currentItem !==null ? currentItem.implicitHeight :0)
            currentIndex: tab.currentIndex

            Repeater {
                id: rep1
                model: priv.model

                Loader {
                    active: swipe.currentIndex === index
                    sourceComponent: modelData
                }
            }
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        Settings {
            id: settings
            category: "qml/linefilter"
            property alias currentIndex : swipe.currentIndex

            Component.onCompleted: {
                if( currentIndex >= swipe.count)
                    tab.currentIndex = swipe.count-1
            }
        }

        Repeater {
            id:rep
            model: priv.titles

            TabButton {
                text : modelData
                width: Math.max(120, tab.width / rep.model.length)
            }
        }
    }
}
