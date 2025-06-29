import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore

ScrollablePage {
    id : page

    Component.onDestruction: cppGlue.saveSettingsInFuture()

    Column {
        spacing:25

        PageIndicator {
            id: pi
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
                model: titleSourceModel

                Loader {
                    active: SwipeView.isCurrentItem || SwipeView.isNextItem || SwipeView.isPreviousItem
                    source: "settingspages/" + model.source
                }
            }
        }
    }

    header: TabBar {
        id : tab
        currentIndex: swipe.currentIndex

        ListModel {
            id: titleSourceModel
            ListElement { title: "General"; source: "General.qml" }
            ListElement { title: "ID colour changes (circle only)"; source: "CircleIDColorChange.qml" }
        }

        Settings {
            id: settings
            category: "qml/settingsPage"
            property alias currentIndex : swipe.currentIndex

            Component.onCompleted: {
                if( currentIndex >= swipe.count)
                    tab.currentIndex = swipe.count-1
            }
        }

        Repeater {
            id:rep
            model: titleSourceModel

            TabButton {
                text : model.title
                width: Math.max(120, tab.width / model.title.length)
            }
        }
    }
}
