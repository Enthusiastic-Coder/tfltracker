import QtQuick
import QtQuick.Controls
import CppClassLib

FlickableDialog {

    onAboutToShow: uiMode.setUserUIActive(true)
    onAboutToHide: drawer.open()
    onAccepted: cppGlue.saveSettingsInFuture()
}
