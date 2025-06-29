import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Flickable {
    id: flick
    Layout.fillWidth: true
    Layout.fillHeight: true
    contentHeight: height
    contentWidth: width

    clip: true
    ScrollBar.vertical: ScrollBar{active: flick.contentHeight/flick.height > 1 ? true : false}
    ScrollBar.horizontal: ScrollBar{active: flick.contentWidth/flick.width > 1 ? true : false}
    boundsBehavior: Flickable.StopAtBounds

    PinchArea {
        id: pinch
        width: Math.max(flick.contentWidth, flick.width)
        height: Math.max(flick.contentHeight, flick.height)

        property real initialWidth
        property real initialHeight

        onPinchStarted: {
            initialWidth = flick.contentWidth
            initialHeight = flick.contentHeight
        }

        onPinchUpdated: {
            var newWidth = initialWidth * pinch.scale
            var newHeight = initialHeight * pinch.scale

            if (newWidth < flick.width || newHeight < flick.height) {
                flick.resizeContent(flick.width, flick.height, Qt.point(flick.width/2, flick.height/2))
            }
            else {
                flick.contentX += pinch.previousCenter.x - pinch.center.x
                flick.contentY += pinch.previousCenter.y - pinch.center.y
                flick.resizeContent(initialWidth * pinch.scale, initialHeight * pinch.scale, pinch.center)
            }
        }

        onPinchFinished: {
            flick.returnToBounds()
        }

        Image {
            id: image
            width: flick.contentWidth
            height: flick.contentHeight
            fillMode: Image.PreserveAspectFit
            source: "qrc:/images/tubeMap.png"
            MouseArea {
                anchors.fill: parent
                onDoubleClicked: {
                    flick.resizeContent(flick.contentWidth*1.5, flick.contentHeight*1.5, Qt.point(mouseX, mouseY))
                }
                onWheel: {
                    if (wheel.angleDelta.y/120*flick.contentWidth*0.1+flick.contentWidth > flick.width && wheel.angleDelta.y/120*flick.contentHeight*0.1+flick.contentHeight > flick.height)
                    {
                        flick.resizeContent(wheel.angleDelta.y/120*flick.contentWidth*0.1+flick.contentWidth, wheel.angleDelta.y/120*flick.contentHeight*0.1+flick.contentHeight, Qt.point(flick.contentX + mouseX, flick.contentY + mouseY))
                        flick.returnToBounds()
                    }
                    else {
                        flick.resizeContent(flick.width, flick.height, Qt.point(flick.width/2, flick.height/2))
                        flick.returnToBounds()
                    }
                }
            }
        }
    }
}


