import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib

SettingsDialog {
    title: "Remove Profile:"

    standardButtons: Dialog.NoButton

    onAccepted: {
        cppGlue.removeRadarProfile(availableProfiles.currentText)
    }

    onAboutToShow: {

        profileList.clear()

        var list = cppGlue.getRadarProfileRemoveList()

        for(var i in list)
            profileList.append( { text: list[i]} )

        availableProfiles.currentIndex = 0
    }

    ColumnLayout {
        width: parent.width

        ListModel {
            id: profileList
        }

        ComboBox {
            id: availableProfiles
            textRole: "text"
            model: profileList
            Layout.fillWidth: true
        }

        RowLayout {
            Layout.preferredWidth: parent.width
            Layout.columnSpan: 2
            spacing: 20
            Button {
                id: saveBtn
                text: "Remove"
                Layout.fillWidth: true
                onClicked: accept()
                Layout.alignment: Qt.AlignRight
                enabled: availableProfiles.currentText.length > 0
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
