import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page
    topPadding: 20

    ColumnLayout {

        SwitchListOptions {
            actions: [ui.action_InAppCheckMe]
            Layout.fillWidth: true
        }

        Rectangle {
            Layout.fillWidth: true;
            height: 5
            border.width: 5
            color: "black"
        }

        Repeater {
            id: inappItem
            model : ui.menu_InAppPurchases.actions

            RowLayout {
                Layout.fillWidth: true;

                ColumnLayout {
                    Layout.fillWidth: true

                    Label {
                        text: modelData.text
                        wrapMode: Label.Wrap
                        Layout.fillWidth: true;
                    }
                    Rectangle {
                        Layout.fillWidth: true;
                        height: 2
                        border.width: 1
                    }
                }
            }
        }

        Button {
            id: monthlyBtn
            text : "Monthly Purchase"
            onClicked: ui.action_Purchase_Monthly_Sub.trigger()
        }
    }
}
