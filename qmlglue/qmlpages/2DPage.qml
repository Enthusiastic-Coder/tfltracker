import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    ColumnLayout {

        DelegateSwitchListOptions {
            actions: ui.menu_2D.actions
            Layout.fillWidth: true
        }
    }
}
