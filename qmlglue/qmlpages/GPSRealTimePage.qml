import QtQuick
import CppClassLib
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    topPadding: 20

    Component.onDestruction: {
        ui.GPSUpdateInterval = updateInterval.text
        ui.GPSHdgCutOffSpeed = minimumSpeed.text
        ui.action_RealTime_GPS.isChecked = realTimeGPS.checked
    }

    Component.onCompleted: {
        updateInterval.text = ui.GPSUpdateInterval
        minimumSpeed.text = ui.GPSHdgCutOffSpeed
        realTimeGPS.text = ui.action_RealTime_GPS.text
        realTimeGPS.checked = ui.action_RealTime_GPS.isChecked
    }

    GridLayout {
        id: grid
        width: parent.width*0.9
        columns:2
        anchors.horizontalCenter: parent.horizontalCenter

        Label {
            text: "You can start and stop this feature from the main radar screen by double-clicking the satellite button top middle of the screen."
            wrapMode: Text.Wrap
            Layout.columnSpan: 2
            Layout.fillWidth: true
            Layout.topMargin: 20
            Layout.bottomMargin: 50

        }

        Switch {
            id: realTimeGPS
            onCheckedChanged: ui.action_RealTime_GPS.isChecked = checked
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Label {
            text:"Update Interval (millisecond):"
            Layout.fillWidth: true
        }

        TextField {
            id: updateInterval
            inputMethodHints: Qt.ImhDigitsOnly
            Layout.fillWidth: true
            placeholderText: "Use 1000"
        }

        Label {
            text:"Minimum Speed (m/s):"
            Layout.fillWidth: true
        }

        TextField {
            id: minimumSpeed
            inputMethodHints: Qt.ImhDigitsOnly
            Layout.fillWidth: true
            placeholderText: "Use 2"
        }

    }
}
