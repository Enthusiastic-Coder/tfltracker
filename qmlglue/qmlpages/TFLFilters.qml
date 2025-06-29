import QtQuick
import QtQuick.Controls

ScrollablePage {
    id : page

    Component.onDestruction: radarView.updateOSM()

    Component.onCompleted: {
        verbBusCombo.currentIndex =  verbBusCombo.find(ui.menu_Label_Bus_Verbosity.checkedAction.text)
        verbTrainCombo.currentIndex = verbTrainCombo.find(ui.menu_Label_Train_Verbosity.checkedAction.text)
        obj.ready = true
    }

    Column {
        QtObject {
            id: obj
            property bool ready : false
        }

        DelegateSwitchListOptions {
            width: parent.width
            actions: ui.menu_TFL_Filter.actions
        }

        Label {
            text: "Bus Label:"
        }

        ComboBox  {
            id: verbBusCombo
            width: parent.width
            model: ui.menu_Label_Bus_Verbosity.actions
            textRole: "text"
            onCurrentIndexChanged: if( obj.ready)
                                       ui.trigger(ui.menu_Label_Bus_Verbosity.actions[currentIndex].objectName)
        }

        Item {
            implicitHeight: 20
            implicitWidth :20
        }

        Label {
            text: "Train Label:"
        }

        ComboBox  {
            id: verbTrainCombo
            width: parent.width
            model: ui.menu_Label_Train_Verbosity.actions
            textRole: "text"
            onCurrentIndexChanged: if( obj.ready)
                                       ui.trigger(ui.menu_Label_Train_Verbosity.actions[currentIndex].objectName)
        }
    }
}
