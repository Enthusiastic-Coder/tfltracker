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
                UnitPage { actions : modelData.actions }
            }
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        QtObject {
            id: obj
            property var model: [ui.menuUnitsSpeed, ui.menuUnitsAltitude, ui.menuUnitsDistance, ui.menuUnitsVsiInterval]
        }

        Settings {
            id: settings
            category: "qml/unitspage"
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
                text : modelData.title
                width: Math.max(120, tab.width / rep.model.length)
            }
        }
    }
}
