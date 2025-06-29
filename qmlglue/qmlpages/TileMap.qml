import QtQuick
import CppClassLib
import QtQuick.Controls
import QtQuick.Layouts

ScrollablePage {
    id : page

    property var tileManager : cppGlue.getMapTileProfiles()

    Component.onDestruction: {
        tileManager.activeProfile = combo.currentText
        tileManager.doUpdate()
    }

    Component.onCompleted: {
        combo.currentIndex = combo.indexOfValue(tileManager.activeProfile)        
    }

    GridLayout {
        id: grid
        width: parent.width * 0.9
        columns:2
        anchors.horizontalCenter: parent.horizontalCenter

        Connections {
            target: tileManager

            function onTilesChanged() {
                combo.model = Object.keys(tileManager.Tiles)
                combo.currentIndex = combo.indexOfValue(tileManager.activeProfile)
            }
        }

        QtObject {
            id:obj

            function getActiveTile() {
                var idx = combo.currentIndex

                if( idx === -1)
                    return null;

                 return tileManager.Tiles[combo.textAt(idx)]
            }
        }

        Label {
            text:"<b>Seek permission from server owners before connecting</b>"
            Layout.topMargin: 20
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        SwitchDelegate {
            id: isActive
            text: "Active"
            checkable: true
            checked : tileManager.active
            onCheckedChanged: tileManager.active = checked
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        SwitchDelegate {
            id: showZoom
            text: "Show Zoom Numbers"
            checkable: true
            checked : tileManager.showZoom
            onCheckedChanged: tileManager.showZoom = checked
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Rectangle {
            height :2
            border.color: "black"
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Dialog {
            id: dialog
            title: add?"Add New Profile":"Remove Profile"
            //standardButtons: Dialog.Ok | Dialog.Cancel

            anchors.centerIn: parent
            parent: Overlay.overlay
            focus: true
            modal: true
            closePolicy: Popup.CloseOnEscape

            property bool add

            contentItem: ColumnLayout {
                spacing: 20
                TextField {
                    id: inputField
                    visible: dialog.add
                    placeholderText: "Enter Profile name"
                    onTextChanged: okBut.enabled = text.length >0
                }

                ComboBox {
                    id: deleteCombo
                    visible: !dialog.add
                    model : Object.keys(tileManager.Tiles)
                    onCurrentIndexChanged: okBut.enabled = currentIndex !==-1
                }

                RowLayout {
                    Button {
                        id: okBut
                        text : "OK"
                        enabled : false
                        onClicked: {
                            if( dialog.add) {
                                tileManager.add(inputField.text)
                                combo.currentIndex = combo.indexOfValue(inputField.text)
                            }
                            else
                                tileManager.remove(deleteCombo.currentText)

                            dialog.visible = false
                        }
                    }
                    Button {
                        text : "Cancel"
                        onClicked: dialog.visible = false
                    }
                }
            }
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.columnSpan: 2

            Button {
                text: "Add"
                onClicked: {
                    dialog.add = true
                    dialog.visible = true
                }
            }

            Button {
                text: "Remove"
                onClicked: {
                    dialog.add = false
                    dialog.visible = true
                }
            }
        }

        Rectangle {
            height :2
            border.color: "black"
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Label {
            text:"Active Profile:"
            Layout.columnSpan: 2
            Layout.fillWidth: true
            wrapMode: Text.Wrap
        }

        ComboBox {
            id: combo
            model : Object.keys(tileManager.Tiles)
            onCurrentIndexChanged: {

                if( currentIndex !== -1) {
                    var tile = tileManager.Tiles[textAt(currentIndex)];

                    urlText.text = tile.url
                    urlText2.text = tile.url2

                    user.text = tile.userName
                    password.text = tile.password

                    //showRunway.checked = tile.showRunway
                    showZoom12.checked = tile.showZoom12
                    showZoom13.checked = tile.showZoom13
                    showZoom14.checked = tile.showZoom14
                }
            }
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        Rectangle {
            height :2
            border.color: "black"
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        // SwitchDelegate {
        //     id: showRunway
        //     text: "Show Runway"
        //     enabled: combo.currentIndex !== -1
        //     onCheckedChanged: {
        //         var tile = obj.getActiveTile()

        //         if( tile !== null)
        //             tile.showRunway = checked
        //     }
        //     checkable: true
        //     Layout.fillWidth: true
        //     Layout.columnSpan: 2
        // }

        SwitchDelegate {
            id: showZoom12
            text: "Show Zoom 12"
            enabled: combo.currentIndex !== -1
            onCheckedChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.showZoom12 = checked
            }
            checkable: true
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        SwitchDelegate {
            id: showZoom13
            text: "Show Zoom 13"
            enabled: combo.currentIndex !== -1
            onCheckedChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.showZoom13 = checked
            }
            checkable: true
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        SwitchDelegate {
            id: showZoom14
            text: "Show Zoom 14"
            enabled: combo.currentIndex !== -1
            onCheckedChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.showZoom14 = checked
            }
            checkable: true
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        Rectangle {
            height :2
            border.color: "black"
            Layout.fillWidth: true
            Layout.columnSpan: 2
        }

        TextArea {
            text: 'URL below takes the form: http[s]//{website}/{params}<p>where {params} specify the format of the URL and provide the arguments {x}, {y} and {zoom}<p>e.g. <u>https://{website}/{zoom}/{x}/{y}.png</u><p> where x, y are tile numbers'
            wrapMode: Text.Wrap
            Layout.fillWidth: true
            textFormat: Text.AutoText
            readOnly: true
            font.pixelSize: 14
            Layout.columnSpan: 2
        }

        Label {
            text:"URL:"
            wrapMode: Text.Wrap
            Layout.fillWidth: true;
            Layout.columnSpan: 2
        }

        TextField {
            id: urlText
            wrapMode: Text.Wrap
            inputMethodHints: Qt.ImhUrlCharactersOnly
            Layout.fillWidth: true;
            Layout.columnSpan: 2
            enabled: combo.currentIndex !== -1
            onTextChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.url = text
            }

        }

        Label {
            text:"URL2 (only required by some servers that add layers):"
            wrapMode: Text.Wrap
            Layout.fillWidth: true;
            Layout.columnSpan: 2
            Layout.topMargin: 20
        }

        TextField {
            id: urlText2
            wrapMode: Text.Wrap
            inputMethodHints: Qt.ImhUrlCharactersOnly
            Layout.fillWidth: true;
            Layout.columnSpan: 2
            enabled: combo.currentIndex !== -1
            onTextChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.url2 = text
            }
        }

        Rectangle {
            height :2
            border.color: "black"
            Layout.fillWidth: true
            Layout.columnSpan: 2
            Layout.topMargin: 20
        }

        Label {
            text:"Enter credentials here if server requires them:"
            wrapMode: Text.Wrap
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        Label {
            id: userLabel
            text:"User:"
            wrapMode: Text.Wrap
        }

        TextField {
            id: user
            enabled: combo.currentIndex !== -1
            onTextChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.userName = text
            }

            wrapMode: Text.Wrap
            inputMethodHints: Qt.ImhNoPredictiveText
            Layout.fillWidth: true
        }

        Label {
            id: passLabel
            text: "Password:"
            wrapMode: Text.Wrap
        }

        TextField {
            id: password
            enabled: combo.currentIndex !== -1
            onTextChanged: {
                var tile = obj.getActiveTile()

                if( tile !== null)
                    tile.password = text
            }

            wrapMode: Text.Wrap
            inputMethodHints: Qt.ImhHiddenText | Qt.ImhSensitiveData
            echoMode: TextInput.PasswordEchoOnEdit
            Layout.fillWidth: true
        }
    }
}
