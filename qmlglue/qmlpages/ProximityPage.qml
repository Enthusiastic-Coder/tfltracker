import QtQuick
import CppClassLib
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    Component.onCompleted: {
        distance.text = ui.ProximityDist

        distanceLabel.text = "Distance (" + ui.unitDist + ")"

        active.checked = ui.action_Proximity_Active.isChecked
        muteSound.checked = ui.action_Proximity_Mute_sound.isChecked
    }

    Component.onDestruction:  {
        ui.ProximityDist = distance.text

        ui.action_Proximity_Active.isChecked = active.checked
        ui.action_Proximity_Mute_sound.isChecked = muteSound.checked
    }

    GridLayout {
        id: grid
        columns:2
        width: parent.width

        Switch {
            id: active
            text: "Active"
            Layout.columnSpan: 2
        }

        Switch {
            id: muteSound
            text: "Mute Sound"
            Layout.columnSpan: 2
        }

        SwitchListOptions {
            actions: [ui.action_Show_Proximity_Rings]
            Layout.columnSpan: 2
            width: parent.width
        }

        Label {
            id: distanceLabel
            text: "Distance:"
            Layout.fillWidth: true
        }

        TextField {
            id: distance
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhDigitsOnly
        }
    }
}
