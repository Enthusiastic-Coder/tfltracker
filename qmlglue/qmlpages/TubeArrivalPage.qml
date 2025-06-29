import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    property var line

    id: col
    spacing: 5

    QtObject {
        id: obj
        property var stns : cppGlue.getTFLLine(line.text).getStopPoints();
    }

    Repeater {
        model: obj.stns

        Button {
            id: but
            text: modelData.displayName
            font.pixelSize: 20
            Layout.preferredWidth: parent.width*0.9
            Layout.alignment: Qt.AlignHCenter
            onClicked:  {
                var params = [modelData.id, modelData.name, "", "", [line.text]]
                stackView.pushPage("qmlpages/ArrivalStatusPage.qml", params)
            }
        }
    }

    //Push items above
    Item {
        implicitHeight: 20
        Layout.fillHeight: true
    }
}
