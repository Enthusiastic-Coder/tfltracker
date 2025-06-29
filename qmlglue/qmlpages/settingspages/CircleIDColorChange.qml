import QtQuick
import CppClassLib
import QtQuick.Controls
import QtQuick.Layouts
import "../"

ColumnLayout{

    Label {
        text: "Hammersmith/Vehicle ID to override to Circle line"
        wrapMode: Text.Wrap
        Layout.fillWidth: true
    }

    DelegateSwitchListOptions {
        actions: ui.menu_Circle_ID_Color_Override.actions
        Layout.fillWidth: true
    }
    Item {
        implicitHeight: 10
        Layout.fillHeight: true
    }

}
