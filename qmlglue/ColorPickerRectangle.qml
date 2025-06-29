import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CppClassLib

Rectangle {
    id: root
    implicitWidth: 30
    implicitHeight: 30
    MouseArea {
        anchors.fill: parent
        onClicked: colorDlg.open()
    }

    Connections {
        target: colorDlg
        function onAccepted(){
            color = colorDlg.color
        }
    }

    Dialog {
        id: colorDlg
        title: "Color Picker:"
        closePolicy: Popup.CloseOnEscape
        modal: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width, parent.height) / 4.0 * 3
        height: parent.height/4.0*3

        parent: Overlay.overlay

        property alias color : finalColor.color
        property real hue : 0.5
        property real saturation : 0.5
        property real brightness : 0.5

        function rgbToHsl(r, g, b) {

            var max = Math.max(r, g, b), min = Math.min(r, g, b)
            var h, s, l = (max + min) / 2
            if (max == min) {
                h = s = 0
            } else {
                var d = max - min
                s = l > 0.5 ? d / (2 - max - min) : d / (max + min)
                switch (max) {
                case r:
                    h = (g - b) / d + (g < b ? 6 : 0)
                    break
                case g:
                    h = (b - r) / d + 2
                    break
                case b:
                    h = (r - g) / d + 4
                    break
                }
                h /= 6;
            }
            return {"h":h, "s":s, "l":l};
        }

        function limitBrightness() {
            if( brightness >= 1.0) brightness = 0.99
            if( brightness <= 0.0) brightness = 0.01
        }

        function updateRValue(value) {
            color = Qt.rgba(value/255, color.g, color.b)
        }

        function updateGValue(value) {
            color = Qt.rgba(color.r, value/255, color.b)
        }

        function updateBValue(value) {
            color = Qt.rgba(color.r, color.g, value/255)
        }

        onColorChanged: {
            var hsl = rgbToHsl(color.r, color.g, color.b)
            hue = hsl.h
            saturation = hsl.s
            brightness = hsl.l

            hueSatCanvas.requestPaint()
            brightnessCanvas.requestPaint()
            indicatorCanvas.requestPaint()
        }

        function updateColor() {

            if( rText.focus || gText.focus || bText.focus)
                return

            color = Qt.hsla(hue, saturation, brightness, 1.0)
            rText.text = Math.floor(color.r*255)
            gText.text = Math.floor(color.g*255)
            bText.text = Math.floor(color.b*255)
        }

        onAboutToShow: color = root.color
        onHueChanged: updateColor()
        onSaturationChanged: updateColor()
        onBrightnessChanged: updateColor()

        GridLayout {
            anchors.fill: parent
            columns: 2

            Canvas {
                id: hueSatCanvas
                Layout.preferredWidth: 300
                Layout.fillWidth: true
                Layout.preferredHeight: 100
                Layout.fillHeight: true

                property int lastX : colorDlg.hue * width
                property int lastY : (1-colorDlg.saturation)*height

                MouseArea {
                    anchors.fill: parent
                    onPressed: focus=true
                    onMouseXChanged: {
                        colorDlg.limitBrightness()
                        colorDlg.saturation = 1 - mouse.y/height
                    }
                    onMouseYChanged: {
                        colorDlg.limitBrightness()
                        colorDlg.hue = mouse.x/width
                    }
                }

                onPaint: {
                    var ctx = getContext('2d')

                    var gradient = ctx.createLinearGradient(0, 0, width, 0)

                    for (var i = 0; i < 10; ++i)
                        gradient.addColorStop(i/10, Qt.hsla(i/10, 1, 0.5, 1));

                    ctx.fillStyle = gradient
                    ctx.fillRect(0, 0, width, height);

                    gradient = ctx.createLinearGradient(0, 0, 0, height)
                    gradient.addColorStop(0, Qt.hsla(0, 0, 0.5, 0));
                    gradient.addColorStop(1, Qt.hsla(0, 0, 0.5, 1));

                    ctx.fillStyle = gradient
                    ctx.fillRect(0, 0, width, height);

                    ctx.lineWidth = 1
                    ctx.strokeStyle = "black"
                    ctx.beginPath()
                    ctx.moveTo(lastX, lastY-10)
                    ctx.lineTo(lastX,lastY+10)

                    ctx.moveTo(lastX-10,lastY )
                    ctx.lineTo(lastX+10, lastY)

                    ctx.closePath()
                    ctx.stroke()
                }
            }

            Item {
                id: brightnessItem
                Layout.preferredWidth: 50
                Layout.fillHeight: true
                Layout.fillWidth: true
                Layout.preferredHeight: 100

                RowLayout {
                    anchors.fill: parent
                    spacing: 0

                    Canvas {
                        id: brightnessCanvas
                        Layout.preferredWidth: parent.width*2/3
                        Layout.preferredHeight: parent.height
                        Layout.fillWidth: true

                        property int lastY: (1-colorDlg.brightness)*height

                        MouseArea {
                            anchors.fill: parent
                            onPressed: focus=true
                            onMouseYChanged: colorDlg.brightness = 1 - mouse.y/height
                        }

                        onPaint: {
                            var ctx = getContext('2d');

                            var gradient = ctx.createLinearGradient(0, 0, 0, height)

                            gradient.addColorStop(0, Qt.hsla(colorDlg.hue, colorDlg.saturation, 1, 1));
                            gradient.addColorStop(0.5, Qt.hsla(colorDlg.hue, colorDlg.saturation, 0.5, 1));
                            gradient.addColorStop(1, Qt.hsla(colorDlg.hue, colorDlg.saturation, 0, 1));

                            ctx.fillStyle = gradient
                            ctx.fillRect(0, 0, width, height);
                        }
                    }

                    Canvas {
                        id: indicatorCanvas
                        Layout.preferredWidth: 20
                        Layout.preferredHeight: parent.height

                        MouseArea {
                            anchors.fill: parent
                            onPressed: focus=true
                            onMouseYChanged: colorDlg.brightness = 1 - mouse.y/height
                        }
                        onPaint: {
                            var ctx = getContext('2d');

                            ctx.fillStyle = "white"
                            ctx.fillRect(0,0, width, height)

                            ctx.lineWidth = 1
                            ctx.fillStyle = "black"
                            ctx.beginPath()
                            ctx.moveTo(0, brightnessCanvas.lastY)
                            ctx.lineTo(10,brightnessCanvas.lastY-10)
                            ctx.lineTo(10,brightnessCanvas.lastY+10)
                            ctx.lineTo(0,brightnessCanvas.lastY)
                            ctx.closePath()
                            ctx.fill()
                        }
                    }
                }
            }

            Rectangle {
                id: finalColor
                Layout.fillWidth: true
                Layout.preferredWidth: parent*2/3
                Layout.preferredHeight: 30
                color: "blue"
                border.width: 1
            }
            Rectangle {
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.columnSpan: 2
                Layout.fillHeight: true
                Layout.preferredHeight: 20

                RowLayout{
                    id: row
                    anchors.fill: parent

                    Label {
                        text: "R:"
                    }

                    TextField {
                        id: rText
                        Layout.preferredWidth: parent.width/4 - row.spacing*2
                        Layout.preferredHeight: parent.height
                        onTextEdited: colorDlg.updateRValue(text)
                        validator : IntValidator {
                            bottom: 0
                            top: 255
                        }
                        inputMethodHints: Qt.ImhPreferNumbers
                    }

                    Label {
                        text: "G:"
                    }

                    TextField {
                        id: gText
                        Layout.preferredWidth: parent.width/4- row.spacing*2
                        Layout.preferredHeight: parent.height
                        onTextEdited: colorDlg.updateGValue(text)
                        validator : IntValidator {
                            bottom: 0
                            top: 255
                        }
                        inputMethodHints: Qt.ImhPreferNumbers
                    }

                    Label {
                        text: "B:"
                    }

                    TextField {
                        id: bText
                        Layout.preferredWidth: parent.width/4- row.spacing*2
                        Layout.preferredHeight: parent.height
                        onTextEdited: colorDlg.updateBValue(text)
                        validator : IntValidator {
                            bottom: 0
                            top: 255
                        }
                        inputMethodHints: Qt.ImhPreferNumbers
                    }
                }
            }

            Item {
                Layout.preferredWidth: parent.width
                Layout.columnSpan: 2
                Layout.fillHeight: true
                Layout.preferredHeight: 20

                RowLayout{
                    anchors.fill: parent
                    Button {
                        text: "OK"
                        onClicked: colorDlg.accept()
                        Layout.preferredHeight: 20
                        Layout.preferredWidth: parent.width/2-parent.spacing
                        Layout.fillHeight: true
                        Layout.alignment: Qt.AlignRight
                    }

                    Button {
                        text: "Cancel"
                        onClicked: colorDlg.reject()
                        Layout.preferredHeight: 20
                        Layout.fillHeight: true
                        Layout.preferredWidth: parent.width/2-parent.spacing
                        Layout.alignment: Qt.AlignLeft
                    }
                }
            }
        }
    }
}
