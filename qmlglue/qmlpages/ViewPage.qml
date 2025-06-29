import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    ColumnLayout {

        RowLayout {
            Layout.topMargin: 20

            Label {
                text:  ui.menuBlip.title
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            ActionListCombBox {
                model: ui.menuBlip.actions
                Layout.fillWidth: true;

            }
        }

        DelegateSwitchListOptions {
            actions: ui.menu_View.actions
            Layout.fillWidth: true
        }
    }
}
