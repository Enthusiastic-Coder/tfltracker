import QtQuick
import QtQuick.Controls
import CppClassLib

Dialog {
    id: dlg
    x: (window.width - width) / 2
    y: (window.height - height) / 2
    width: window.width / 5.0 * 4
    contentHeight: Math.min( pane.height, window.height / 5.0 * 3)

    parent: Overlay.overlay

    focus: true
    modal: true
    closePolicy: Popup.CloseOnEscape

    default property alias content: pane.contentItem

    Flickable {
        id: flickable
        anchors.fill: parent
        contentHeight: pane.implicitHeight
        flickableDirection: Flickable.AutoFlickIfNeeded
        clip: true

        Pane {
            id: pane
            width: parent.width
        }

        ScrollIndicator.vertical: ScrollIndicator {
            active: true
            parent: dlg.contentItem
            anchors.top: flickable.top
            anchors.bottom: flickable.bottom
            anchors.right: parent.right
            anchors.rightMargin: -dlg.rightPadding + 1
        }
    }
}
