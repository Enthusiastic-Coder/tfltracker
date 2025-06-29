import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib
import "../"

FlickableDialog {
    title: "Select Lines:"
    property var lines
    property var lineWanted : []
    width: parent.width *9/10
    closePolicy: Popup.CloseOnEscape| Popup.CloseOnPressOutside

    Column {

        QtObject {
            id: obj
        }

        Rectangle {
            width: parent.width
            height: 4
            border.width: 2
            border.color: "black"
        }

        Repeater {
            model: lines

            Column {

                width: parent.width

                CheckBox {
                    text: modelData
                    font.pixelSize: 22
                    font.bold: true
                    checked: lineWanted.indexOf(modelData) !== -1
                    onToggled: {
                        if( checked) {

                            if( lineWanted.indexOf(modelData) === -1)
                                lineWanted.push( modelData)

                        } else {

                            for( var i = 0; i < lineWanted.length; i++)
                               if ( lineWanted[i] === modelData)
                                 lineWanted.splice(i, 1)
                        }
                    }
                }

                Rectangle {
                    width: parent.width
                    height: 2
                    border.width: 2
                    border.color: "black"
                }
            }
        }
    }
}
