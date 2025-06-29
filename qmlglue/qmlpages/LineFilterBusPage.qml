import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtCore
import "../"
import CppClassLib

ColumnLayout {
    width: parent.width
    id: col

    function routeDownloaded(lineid) {

        d.optionEnabled = true

        d.addBusForRebuild(lineid);

        if( d.wantBuildBusList ) {

            ui.busList.push(lineid)
            d.buildBusList()
            d.wantBuildBusList = false
        }

        var line = cppGlue.getTFLLine(lineid)
        line.setUpdatedOK(true);
    }

    function routeFailedToDownload(lineid) {

        d.optionEnabled = true

        var line = cppGlue.getTFLLine(lineid)
        line.setUpdatedOK(false);
    }

    QtObject {
        id: d
        property var menu
        property bool optionEnabled : true
        property bool wantBuildBusList: false
        property var busesToRebuild : []

        function buildBusList() {
            cppGlue.buildBusList()
            loader.sourceComponent = null
            menu = ui.lineGroupBusList
            loader.sourceComponent = busComponent
        }

        function addBusForRebuild(lineid) {

            if( busesToRebuild.indexOf(lineid) == -1)
                busesToRebuild.push(lineid);
        }
    }

    Component.onDestruction: {
        cppGlue.rebuildBuses(d.busesToRebuild);
        cppGlue.onTflRouteDownloaded.disconnect(routeDownloaded)
        cppGlue.onTflRouteFailedToDownload.disconnect(routeFailedToDownload)
    }

    Component.onCompleted: {

        cppGlue.onTflRouteDownloaded.connect(routeDownloaded)
        cppGlue.onTflRouteFailedToDownload.connect(routeFailedToDownload)

        loaderBus.sourceComponent = busHeaderComponent
        d.buildBusList();
    }

    Column {
        Layout.fillWidth: true
        spacing: 10

        Loader {
            width: parent.width
            id: loaderBus
        }

        Loader {
            width: parent.width
            id: loader
        }
    }

    Component {
        id:busHeaderComponent

        RowLayout {
            width: parent.width*0.9

            TextField {
                id: busText
                placeholderText: "Bus number"
                inputMethodHints: Qt.ImhNoPredictiveText
                Layout.fillWidth: true
            }

            Button {
                text: "Add"
                onClicked: {

                    if( fetchListTooBig())
                        return


                    if( !cppGlue.doesBusRouteExist(busText.text.toLowerCase())) {
                        toast.title = "Active Lines"
                        toast.message = busText.text + " does NOT exist."
                        toast.open()
                        return
                    }

                    var found = ui.busList.find( function(e) {
                        return e === busText.text.toLowerCase() })

                    if( found) {
                        toast.title = "Active Lines"
                        toast.message = busText.text + " already exists."
                        toast.open()
                    } else {

                        var line = cppGlue.getTFLLine(busText.text.toLowerCase())

                        if( line != null) {
                            ui.busList.push(busText.text.toLowerCase())
                            d.buildBusList()
                        } else {
                            d.optionEnabled = false
                            d.wantBuildBusList = true
                            cppGlue.downloadBusRoute(busText.text.toLowerCase())
                        }
                    }

                    busText.clear()
                }
            }
        }
    }

    Component {
        id:busComponent

        Column {
            width: parent.width-15
            spacing: 2
            rightPadding: 10

            Rectangle {
                width: parent.width-15
                height: 4
                border.width: 1
            }

            PageIndicator {
                id: pi
                width: parent.width-15
                count: tab.count
                currentIndex: swipe.currentIndex
                anchors.horizontalCenter: parent.horizontalCenter
            }

            TabBar {
                id : tab
                currentIndex: swipe.currentIndex
                width: parent.width-15

                Settings {
                    id: settings
                    category: "qml/lineFilterBusPage"
                    property alias currentIndex : swipe.currentIndex

                    Component.onCompleted: {
                        if( currentIndex >= swipe.count)
                            tab.currentIndex = swipe.count-1
                    }
                }

                Repeater {
                    id:rep
                    model: d.menu

                    TabButton {
                        text : modelData.title
                        width: Math.max(120, tab.width / rep.model.length)
                    }
                }
            }

            SwipeView {
                id : swipe
                width: page.width
                height: Math.max(page.height-tab.height, currentItem !==null ? currentItem.implicitHeight :0)
                currentIndex: tab.currentIndex

                Repeater {
                    model: d.menu

                    Loader {
                        active: SwipeView.isCurrentItem
                        sourceComponent: Column {

                            Repeater {
                                model: modelData.actions
                                Column {
                                    width: parent.width-15
                                    id: col
                                    spacing: 5
                                    rightPadding: 10

                                    RowLayout {
                                        width: parent.width

                                        Switch {
                                            id: control
                                            text:modelData.data.name
                                            checked: modelData.data.Visible
                                            //Layout.preferredWidth: parent.width/3-15

                                            contentItem: Text {
                                                text: control.text
                                                font: control.font
                                                opacity: enabled ? 1.0 : 0.3
                                                color: color
                                                verticalAlignment: Text.AlignVCenter
                                                leftPadding: control.indicator.width + control.spacing
                                            }

                                            onToggled: {

                                                if( checked && fetchListTooBig())
                                                    checked = false

                                                modelData.data.Visible = checked
                                            }
                                        }

                                        RoundButton {
                                            text: checked? "[-]":"[+]"
                                            checkable: true
                                            checked: modelData.data.ShowStops
                                            onCheckedChanged: modelData.data.ShowStops = checked
                                        }

                                        RoundButton {
                                            id: editMode
                                            text: checked? "[e-]":"[e+]"
                                            checkable: true
                                        }

                                        Item {
                                            implicitWidth: 20
                                            Layout.fillWidth: true
                                        }

                                        ColorPickerRectangle {
                                            id: colorPicker
                                            color: modelData.data.Color
                                            border.width: 1
                                            enabled: !modelData.data.ReadOnly
                                            onColorChanged: modelData.data.Color = color
                                            Layout.alignment: Qt.AlignLeft

                                        }

                                        SpinBox {
                                            id: offSet
                                            visible: !modelData.data.ReadOnly
                                            stepSize: 5
                                            editable: false
                                            from: 1
                                            to: 200
                                            value: modelData.data.OffSet
                                            onValueModified: modelData.data.OffSet = parseInt(value)
                                            wrap: Text.Wrap
                                        }
                                    }

                                    RowLayout {
                                        visible: editMode.checked
                                        width: parent.width
                                        spacing: 5

                                        Label {
                                            id: msg
                                            text:  modelData.data.downloadStatusDescription
                                            color: modelData.data.downloadedOK ? "green" : "red"
                                            wrapMode: Text.Wrap
                                            Layout.fillWidth: true
                                        }

                                        Item {
                                            Layout.fillWidth: true
                                            height: 1
                                        }

                                        RoundButton {
                                            text: "Update"
                                            Layout.alignment: Qt.AlignLeft
                                            enabled: d.optionEnabled
                                            onClicked: {
                                                d.optionEnabled = false
                                                cppGlue.downloadBusRoute(modelData.data.name.toLowerCase())
                                            }
                                        }

                                        RoundButton {
                                            Layout.alignment: Qt.AlignLeft
                                            enabled: d.optionEnabled

                                            Dialog {
                                                id: yesNoRemoveDialog
                                                title: "Bus remove:[" + modelData.data.name + "]"

                                                x: (window.width - width) / 2
                                                y: (window.height - height) / 2
                                                width: window.width / 5.0 * 3

                                                parent: Overlay.overlay

                                                focus: true
                                                modal: true
                                                closePolicy: Popup.CloseOnEscape

                                                standardButtons: Dialog.Yes | Dialog.No

                                                onAccepted: {

                                                    var line = cppGlue.getTFLLine(modelData.data.name.toLowerCase())

                                                    if( line !== null ) {

                                                        line.Visible = false
                                                        ui.busList.splice( ui.busList.indexOf(modelData.data.name.toLowerCase()), 1);
                                                        d.buildBusList();
                                                    }
                                                }

                                                Label {
                                                    width: parent.width
                                                    wrapMode: Text.Wrap
                                                    text: "Are you sure you want to remove Bus route [" + modelData.data.name + "] ?"
                                                }
                                            }

                                            text: "Remove"
                                            visible: modelData.data.isBus
                                            onClicked: yesNoRemoveDialog.open()
                                        }
                                    }

                                    Rectangle {
                                        width: parent.width
                                        height: 2
                                        border.width: 1
                                    }

                                }
                            }
                        }
                    }
                }
            }
        }
    }

    //Push items above
    Item {
        implicitHeight: 20
        Layout.fillHeight: true
    }
}
