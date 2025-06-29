import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib

SettingsDialog {
    title: "Enter new profile name:"

    standardButtons: Dialog.NoButton

    onAccepted: {
        /// Profile must be non-blank *****
        cppGlue.addRadarProfile(profileName.text, profileList.get(availableProfiles.currentIndex).data)
    }

    onAboutToShow: {

        profileList.clear()

        for( var i = RadarTemplate.Start+1; i < RadarTemplate.End; ++i) {
            profileList.append({ text: cppGlue.getRadarTemplateString(i), data:i})

        }

        availableProfiles.currentIndex = 0
    }

    GridLayout {
        width: parent.width
        columns:2

        ListModel {
            id: profileList
        }

        TextField {
            id: profileName

            Layout.columnSpan: 2
            Layout.fillWidth: true
            inputMethodHints: Qt.ImhNoPredictiveText
        }

        Label {
            text: "Using profile:"
            Layout.columnSpan: 2
        }

        ComboBox {
            id: availableProfiles
            textRole: "text"
            model: profileList
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.preferredWidth: parent.width
            Layout.columnSpan: 2
            spacing: 20
            Button {
                id: saveBtn
                text: "Save"
                Layout.fillWidth: true
                onClicked: accept()
                Layout.alignment: Qt.AlignRight
                enabled: profileName.text.length > 0
            }
            Button {
                text: "Cancel"
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignLeft
                onClicked: reject()
            }
        }
    }
}
