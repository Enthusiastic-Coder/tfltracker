import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    Component.onDestruction: radarView.updateOSM()   

    ColumnLayout {
        width: parent.width

        DelegateSwitchListOptions {
            actions: ui.menu_Labels_Filter.actions
            Layout.fillWidth: true
        }
    }
}
