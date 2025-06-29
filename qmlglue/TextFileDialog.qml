import QtQuick
import QtQuick.Controls
import CppClassLib

Dialog {
    id: textFileDialog
    title: "Not Set"
    property string filename

    width: window.width / 10.0 * 9
    height:window.height / 10.0 * 9

    x: (window.width - width) / 2
    y: (window.height - height) / 2

    parent: Overlay.overlay

    focus: true
    modal: true
    closePolicy: Popup.CloseOnEscape

    Component {
        id : fileLoaderComponent
        Text {
            id : textArea
            wrapMode: Text.WordWrap
            width: parent.width
            text : cppGlue.textFromFile(textFileDialog.filename)
            textFormat: Text.AutoText

            onLinkActivated: (link) => Qt.openUrlExternally(link)

            Component.onCompleted: {
                text = text.replace("[%CreditInfo]", cppGlue.creditInfo())
                text = text.replace("[%CurrentYear]", cppGlue.currentYear() )
                text = text.replace("[%OpenGLInfo]", cppGlue.getOpenGLVersion())
            }
        }
    }

    onOpened: loader.sourceComponent = fileLoaderComponent
    onClosed: loader.sourceComponent = null

    onAboutToShow: uiMode.setUserUIActive(true)
    onAboutToHide: uiMode.setUserUIActive(false)

    Flickable {
        width: parent.width
        height: parent.height - but.height
        contentHeight: loader.implicitHeight
        clip: true

        Loader {
            id:loader
            width: parent.width
        }

        ScrollIndicator.vertical: ScrollIndicator {
            active: true
        }
    }

    Button {
        id: but
        text: "Close"
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        onClicked: accept()
    }
}
