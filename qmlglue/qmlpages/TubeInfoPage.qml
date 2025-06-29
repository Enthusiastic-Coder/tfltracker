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

            LinesModeStatusPage { mode: "tube,overground,dlr,elizabeth-line,tram"}
            TubeDisruptionPage {}
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
            category: "qml/tubeInfoPage"
            property alias currentIndex : swipe.currentIndex

            Component.onCompleted: {
                if( currentIndex >= swipe.count)
                    tab.currentIndex = swipe.count-1
            }
        }

        Repeater {
            id:rep
            model: ["Status", "Disruption"]

            TabButton {
                id: control
                text : modelData
                width: Math.max(120, tab.width / rep.model.length)
            }
        }
    }
}
