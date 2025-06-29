import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import QtCore
import "../"

ScrollablePage {
    id : page

    Component.onDestruction: {
        cppGlue.updateNationalRailList()
    }

    GenericLinePage {
        id: rep
        line_group: ui.lineGroup_river_bus
    }
}



