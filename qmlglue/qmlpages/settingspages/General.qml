import QtQuick
import CppClassLib
import QtQuick.Controls
import QtQuick.Layouts
import "../"

ColumnLayout{

    DelegateSwitchListOptions {
        actions: ui.menu_Settings_General.actions
        Layout.fillWidth: true
    }
    Item {
        implicitHeight: 10
        Layout.fillHeight: true
    }

}
