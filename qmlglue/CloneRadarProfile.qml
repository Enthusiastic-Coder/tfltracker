import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib

SettingsDialog {
    id: dlg
    title: "Enter new profile name:"

    onAccepted: {
        /// Profile must be non-blank *****
        cppGlue.cloneRadarProfile(profileName.text, profileList.get(availableProfiles.currentIndex).text)
    }

    onAboutToShow: {

        profileList.clear()

        for(var i in ui.menu_2D_Radar_Profiles.actions)
            profileList.append( { text: ui.menu_2D_Radar_Profiles.actions[i].text} )

        availableProfiles.currentIndex = availableProfiles.find(cppGlue.getCurrentRadarProfileName());
    }

    ColumnLayout {
        width: dlg.width

        ListModel {
            id: profileList
        }

        TextField {
            id: profileName
            inputMethodHints: Qt.ImhNoPredictiveText
            Layout.fillWidth: true
        }

        Label {
            text: "Using profile:"
        }

        ComboBox {
            id: availableProfiles
            textRole: "text"
            model: profileList
            Layout.fillWidth: true
        }

    }
}
