import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    ColumnLayout {

        Button {
            text: "Toggle VR"
            onClicked: ui.action_3D_VR.trigger()
            Layout.rightMargin: 20
            Layout.alignment: Qt.AlignRight
        }

        RowLayout {
            Layout.topMargin: 20
            Layout.rightMargin: 20

            Label {
                text:  ui.menuBlip_Verbosity_3D.title
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            ActionListCombBox {
                model: ui.menuBlip_Verbosity_3D.actions
                Layout.fillWidth: true;
            }
        }        

        RowLayout {
            Layout.topMargin: 20
            Layout.rightMargin: 20

            Label {
                text:  ui.actionGroup_skyLineGroup.title
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
            }

            ActionListCombBox {
                model: ui.actionGroup_skyLineGroup.actions
                Layout.fillWidth: true;
            }
        }

        DelegateSwitchListOptions {
            actions: ui.menu_3D.actions
            Layout.fillWidth: true
        }
    }
}
