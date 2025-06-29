import QtQuick
import QtQuick.Controls
import "Helper.js" as Helper

ComboBox {

    textRole: "text"

    QtObject{
        id: ob
        property bool ready: false
    }

    Component.onCompleted: {

        currentIndex = Helper.indexOfCheckedAction(model)
        ob.ready = true
    }
    onCurrentIndexChanged: if( ob.ready )
                               ui.trigger(model[currentIndex].objectName)
}
