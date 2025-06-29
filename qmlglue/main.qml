import QtQuick
import QtQuick.Window 2.11
import QtQuick.Layouts
import QtQuick.Controls.Material 2.1
import QtQuick.Controls
import CppClassLib
import QtCore


ApplicationWindow {
    id: window
    visible: true
    width: 640
    height: 480
    title: "London TFL Tracker"

    required property var builtInStyles


    onClosing: console.log( "onClosing")

    QtObject {
        id: uiMode

        function setUserUIActive(active) {

            cppGlue.setUserUIActive(active)

            if( !active)
                radarView.focus = true
        }
    }

    Settings {
        id: settings
        category: "qml"
        property string style: "Default"
        property alias width: window.width
        property alias height: window.height
    }

    Shortcut {
        sequences: ["Esc", "Back"]
        enabled: stackView.depth > 1
        onActivated: {
            stackView.pop()
            listView.currentIndex = -1
            uiMode.setUserUIActive(false)
        }
    }

    Shortcut {
        sequence: "Menu"
        onActivated: optionsMenu.open()
    }

    Dialog {
        id: toast
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: Math.min(parent.width, parent.height) / 3 * 2

        title:"Toast"
        parent: window.overlay
        modal: true
        closePolicy: Popup.CloseOnEscape

        focus: true
        property string message

        onAboutToShow: msg.text = message

        ColumnLayout {
            width: parent.width

            Label {
                id:msg
                Layout.preferredWidth: parent.width
                wrapMode: Text.Wrap
            }

            Button {
                text:"Close"
                onClicked: toast.accept()
                Layout.alignment: Qt.AlignRight
                Layout.preferredWidth: parent.width/2
            }
        }
    }

    StackView {
        id: stackView
        anchors.fill: parent
        focus:true

        function pushPage(qmlPage, args){

            stackView.push( "qrc:/qmlglue/" + qmlPage, {qmlArgs:args} )
            drawer.close()
            uiMode.setUserUIActive(true)
        }

        initialItem: TFLView {
            id : radarView

//            Button {
//                text:"DialogTest"
//                onClicked: {
//                    toast.title = "TESTING WIDTH"
//                    toast.message = "DialogTesting................"
//                    toast.open()

//                }
//            }

            onWantShowMaximum: {
                header.visible=false
                window.showFullScreen()
            }
            onWantShowNormal : {
                header.visible=true
                window.showMaximized()
            }

            onShowMenuOptions : {
                header.visible=true
                if( window.visibility == Window.FullScreen)
                    window.showMaximized();
                drawer.open()
            }

            onShowQMLPage : (page,cppArgs) => stackView.pushPage(page, cppArgs)

            onShowQMLSelectStopPointPage : (cppArgs) => {
                dynamicLoadDialog.sourceComponent = selectStopPointComponent
                dynamicLoadDialog.item.qmlArgs = cppArgs
            }

            Component.onCompleted:  {

                cppGlue.onShowReleaseNotes.connect( function() { textFileDialog.open() } )

                cppGlue.onCompleted.connect( function () {
                    // Build any menus required
                    obj.populate()
                    listView.model = obj.menuModel
                    listView.currentIndex = -1
                });

                cppGlue.onQmlPopUp.connect( function(title, msg) {
                    toast.title = title
                    toast.message = msg
                    toast.open()
                })

                cppGlue.onQmlPromptProductPurchase.connect( function(msg, productID) {
                    inAppPurchaseDialog.productID = productID;
                    inAppPurchaseDialog.msg = msg
                    inAppPurchaseDialog.open()
                } );

                cppGlue.onShowQMLPage.connect( function( page, cppArgs)  {
                     stackView.pushPage(page, cppArgs)
                });

                cppGlue.setViewFrameBuffer(this)

            }
        }
    }

    header: ToolBar {
        Material.foreground: "white"

        onVisibleChanged: radarView.tellFullScreen(!visible)

        RowLayout {
            spacing: 20
            anchors.fill: parent

            ToolButton {
                id: toolButton
                focusPolicy: Qt.NoFocus
                icon.name: stackView.depth > 1 ? "back" : "drawer"
                onClicked: {
                    if (stackView.depth > 1) {
                        stackView.pop()

                        if(stackView.depth ===1 ) {
                            uiMode.setUserUIActive(false)
                            listView.currentIndex = -1
                        }
                    } else {
                        drawer.open()
                    }
                }
            }

            Label {
                text: listView.currentItem ? listView.currentItem.text : window.title
                font.pixelSize: 20
                Layout.fillWidth: true
            }

            /*Button {
                    text: "Toggle Radar View"
                    onClicked: {
                    radarView.hideRadarView(2000);
                }
            }

            Button {
                text: "PersistenceOff"
                onClicked: {
                    radarView.persistenceOff();
                }
            }*/

            ToolButton {
                icon.name: "menu"
                focusPolicy: Qt.NoFocus
                onClicked: optionsMenu.open()

                Menu {
                    id: optionsMenu
                    x: parent.width - width
                    transformOrigin: Menu.TopRight

                    MenuItem {
                        text: "Credits"
                        onTriggered: creditFileDialog.open()
                    }
                    MenuItem {
                        text: "Graphics"
                        onTriggered: graphicsDriverDialog.open();
                    }
                    MenuItem {
                        text: "Instructions"
                        onTriggered: instructionsDialog.open()
                    }
                    MenuItem {
                        text: "Release Notes"
                        onTriggered: textFileDialog.open()
                    }
                    MenuItem {
                        text: "Disclaimer"
                        onTriggered: disclaimerDialog.open()
                    }
                    MenuItem {
                        text: "Style"
                        onTriggered: styleDialog.open()
                    }
                }
            }
        }
    }

    Drawer {
        id: drawer
        width: Math.min(window.width, window.height) / 3 * 2
        height: window.height
        interactive: opened

        onAboutToShow: uiMode.setUserUIActive(true)
        onAboutToHide: uiMode.setUserUIActive(false)

        QtObject {
            id: obj

            property var menuModel: ListModel {}

            function populate() {
                menuModel.append({ title: "Purchase Options", source: "PurchasesPage.qml" })

                if( cppGlue.isMe())
                    menuModel.append({ title: "Purchase Test", source: "PurchasesTest.qml" })

                menuModel.append({ title: "Proximity", source: "ProximityPage.qml" })
                menuModel.append({ title: "GPS RealTime", source: "GPSRealTimePage.qml" })
                menuModel.append({ title: "Estimated Position", source: "LinesFilterPage.qml" })
                menuModel.append({ title: "National Rail", source: "NationalRailPage.qml" })
//              menuModel.append({ title: "River Bus", source: "RiverBoatPage.qml" })

                //menuModel.append( { title: "2D", source: "2DPage.qml" })
                menuModel.append({ title: "3D", source: "3DPage.qml" })
                menuModel.append({ title: "Map Tiles", source:"TileMap.qml" })
                menuModel.append( { title: "Map Filters", source: "LabelFilters.qml" })
                menuModel.append( { title: "TFL Filters", source: "TFLFilters.qml" })
                menuModel.append({ title: "Arrivals", source: "TubeArrivalsPage.qml" })
                menuModel.append({ title: "Status", source: "TubeInfoPage.qml" })
                menuModel.append({ title: "Tube Map", source: "TubeMapPage.qml" })
                menuModel.append({ title: "Units", source: "UnitsPage.qml" })
//                menuModel.append( { title: "Color Profiles" })
//                menuModel.append( { title: "View", source: "ViewPage.qml" })
                menuModel.append( { title: "Settings", source: "SettingsPage.qml" })
                menuModel.append( { title: "Quit App"})
            }
        }

        ListView {
            id: listView

            currentIndex: -1
            anchors.fill: parent

            delegate: ItemDelegate {
                width: parent.width
                text: model.title
                highlighted: ListView.isCurrentItem

                onClicked: {
                    if( model.source != null ) {
                        listView.currentIndex = index
                        if( stackView.depth === 1 && model.source.length >0) {
                            stackView.push("qrc:/qmlglue/qmlpages/"+model.source)
                            drawer.close()
                            uiMode.setUserUIActive(true)
                            return
                        }
                    }

                    drawer.close()
                    uiMode.setUserUIActive(true)

                    listView.currentIndex = -1;

                    if( model.title === "Quit App"){
                        window.close()
                    }
                    else if( model.title === "Radar Profiles") {
                        cppGlue.updateRadarProfileMenus()
                        menuLoader.sourceComponent = null
                        menuLoader.sourceComponent = radarProfileMenuComponent
                    }
                    else
                    {
                        uiMode.setUserUIActive(false)
                    }
                }
            }

            model: ListModel {
                ListElement { title: "...loading.....";}
                ListElement { title: "Quit App"}
            }

            ScrollIndicator.vertical: ScrollIndicator {
                active: true
            }
        }
    }

    Component {
        id: radarProfileMenuComponent

        Menu {
            id : radarProfilesMenu
            signal finished

            onAboutToShow: uiMode.setUserUIActive(true)
            onAboutToHide: uiMode.setUserUIActive(false)

            ButtonGroup { id: radarProfilesGroup }

            QtObject {
                id:priv
                function buildMenuItem(parent, menu, group) {
                    for( var k in menu.actions ) {

                        var subMenuAction = menu.actions[k]

                        var subMenuItemObject = radarProfilesMenuItem.createObject(window, {
                                                                                       text: subMenuAction.text,
                                                                                       objectName: subMenuAction.objectName,
                                                                                       checked: subMenuAction.isChecked,
                                                                                       checkable : group !== undefined,
                                                                                       enabled: !subMenuAction.isDisabled
                                                                                   })

                        if( group ) {
                            subMenuItemObject.ButtonGroup.group = radarProfilesGroup
                        }

                        parent.addItem(subMenuItemObject);
                    }
                }
            }

            function buildMenuItems() {

                var profilesMenu = componentMenu.createObject(window, { title: "Select"} );

                priv.buildMenuItem(profilesMenu, ui.menu_2D_Radar_Profiles, true)
                addMenu(profilesMenu)

                priv.buildMenuItem(radarProfilesMenu, ui.menu_Radar_Profiles,false)
            }

            Component {
                id: radarProfilesMenuItem
                MenuItem {
                    onTriggered: {

                        switch(objectName) {

                        case ui.action_2D_Radar_Profiles_Add.objectName:
                            dynamicLoadDialog.sourceComponent = addProfileComponent
                            finished()
                            break

                        case ui.action_2D_Radar_Profiles_Clone.objectName:
                            dynamicLoadDialog.sourceComponent = cloneProfileComponent
                            finished()
                            break

                        case ui.action_2D_Radar_Profiles_Edit.objectName:
                            dynamicLoadDialog.sourceComponent = editRadarProfileComponent
                            finished()
                            break

                        case ui.action_2D_Radar_Export_Profile.objectName:
                            dynamicLoadDialog.sourceComponent = exportProfileComponenet
                            finished()
                            break

                        case ui.action_2D_Radar_Import_Profile.objectName:
                            dynamicLoadDialog.sourceComponent = importProfileComponent
                            finished()
                            break

                        case ui.action_2D_Radar_Profiles_Remove.objectName:
                            dynamicLoadDialog.sourceComponent = removeProfileComponent
                            finished()
                            break

                        default:
                            ui.trigger(objectName)
                            uiMode.setUserUIActive(false)
                            finished()

                        }
                    }
                }
            }
            Component {
                id: componentMenu
                Menu {
                    onAboutToShow: uiMode.setUserUIActive(true)
                    onAboutToHide: uiMode.setUserUIActive(false)
                }
            }
        }
    }

    Loader {
        id: menuLoader
        onStatusChanged: {
            if( item != null) {
                item.buildMenuItems()
                item.open()
            }
        }
        Connections {
            target: menuLoader.item
            function onFinished() {
                menuLoader.sourceComponent = null
            }
        }
    }

    Loader {
        id: dynamicLoadDialog
        onStatusChanged: if( item != null) item.open()
        Connections {
            target: dynamicLoadDialog.item
            function onFinished() {
                dynamicLoadDialog.sourceComponent = null
            }
        }
    }

    Component {
        id: selectStopPointComponent
        SelectStationPointsDialog {
            signal finished()
            onAccepted: finished()
            onRejected: finished()
        }
    }

    Component {
        id: addProfileComponent
        AddRadarProfileDialog {
            signal finished()
            onAccepted: finished()
            onRejected: finished()
        }
    }

    Component {
        id: cloneProfileComponent
        CloneRadarProfile {
            signal finished()
            onAccepted: finished()
            onRejected: finished()
        }
    }

    Dialog {
        id: styleDialog
        x: Math.round((window.width - width) / 2)
        y: Math.round(window.height / 6)
        width: Math.round(Math.min(window.width, window.height) / 3 * 2)
        modal: true
        focus: true
        title: "Settings"

        standardButtons: Dialog.Ok | Dialog.Cancel

        onAboutToShow: uiMode.setUserUIActive(true)
        onAboutToHide: uiMode.setUserUIActive(false)

        onAccepted: {
            settings.style = styleBox.displayText
            styleDialog.close()
        }
        onRejected: {
            styleBox.currentIndex = styleBox.styleIndex
            styleDialog.close()
        }

        contentItem: ColumnLayout {
            id: settingsColumn
            spacing: 20

            RowLayout {
                spacing: 10

                Label {
                    text: "Style:"
                }

                ComboBox {
                    id: styleBox
                    property int styleIndex: -1
                    model: window.builtInStyles
                    Component.onCompleted: {
                        styleIndex = find(settings.style, Qt.MatchFixedString)
                        if (styleIndex !== -1)
                            currentIndex = styleIndex
                    }
                    Layout.fillWidth: true
                }
            }

            Label {
                text: "Restart required"
                color: "#e41e25"
                opacity: styleBox.currentIndex !== styleBox.styleIndex ? 1.0 : 0.0
                horizontalAlignment: Label.AlignHCenter
                verticalAlignment: Label.AlignVCenter
                Layout.fillWidth: true
                Layout.fillHeight: true
            }
        }
    }

    TextFileDialog {
        id: textFileDialog
        title : "Release Notes:"
        filename: ":/qmlglue/ReleaseNotes.html"
    }

    TextFileDialog {
        id: creditFileDialog
        title : "Credits:"
        filename: ":/qmlglue/CreditsPage.html"
    }

    TextFileDialog {
        id: disclaimerDialog
        title : "Disclaimer:"
        filename: ":/qmlglue/DisclaimerPage.html"
    }

    TextFileDialog {
        id: instructionsDialog
        title : "Some help/instructions:"
        filename: ":/qmlglue/InstructionsPage.html"
    }

    TextFileDialog {
        id: graphicsDriverDialog
        title : "Graphics Information:"
        filename: ":/qmlglue/GraphicsDriverPage.html"
    }

    Dialog {
        id : inAppPurchaseDialog
        title : "Playstore Purchase"

        property string msg
        property string productID

        x: (window.width - width) / 2
        y: (window.height - height) / 2
        width: window.width / 5.0 * 3

        parent: Overlay.overlay

        focus: true
        modal: true
        closePolicy: Popup.CloseOnEscape

        standardButtons: Dialog.Yes | Dialog.No

        onAccepted: cppGlue.inAppPurchase(productID)
        onAboutToShow: uiMode.setUserUIActive(true)
        onAboutToHide: uiMode.setUserUIActive(false)

        Label {
            width: parent.width
            wrapMode: Text.Wrap
            text: inAppPurchaseDialog.msg
        }
    }

    function fetchListTooBig() {
        var fetchCount = cppGlue.getFetchListCount()
        var maxFetchCount = cppGlue.getFetchListLimit()

        if( fetchCount >= maxFetchCount) {
            toast.title = "Active Lines"
            toast.message = "Max of " + maxFetchCount +" active lines allowed."
            toast.open()
            return true
        }

        return false
    }

}
