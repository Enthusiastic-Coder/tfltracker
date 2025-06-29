import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore

ScrollablePage {
    id : page

    Column {
        PageIndicator {
            id : pi
            count: tab.count
            currentIndex: swipe.currentIndex
            anchors.horizontalCenter: parent.horizontalCenter
        }

        SwipeView {
            id : swipe
            width: parent.width
            height: Math.max(page.height-tab.height-pi.height, currentItem !==null ? currentItem.implicitHeight :0)
            currentIndex: tab.currentIndex

            Repeater {
                model: obj.model

                Loader {
                    active: swipe.currentIndex === index
                    sourceComponent: TubeArrivalPage { line : modelData }
                }
            }
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        QtObject {
            id: obj
            property var model: ui.lineGroup_tube.actions
        }

        Settings {
            id: settings
            category: "qml/tubeArrivalPage"
            property alias currentIndex : swipe.currentIndex

            Component.onCompleted: {
                if( currentIndex >= swipe.count)
                    tab.currentIndex = swipe.count-1
            }
        }

        Repeater {
            id:rep
            model: obj.model

            TabButton {
                id: control
                text : modelData.text
                width: Math.max(120, tab.width / rep.model.length)

                background: Rectangle {
                    color: modelData.data.Color
                }
            }
        }
    }
}
