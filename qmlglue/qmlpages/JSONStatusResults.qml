import QtQuick
import QtQuick.Controls
import CppClassLib
import QtCore
import QtQuick.Layouts

Component {
    Column {
        Repeater {
            model : obj.model

            GridLayout {
                property string lineId : modelData["id"]
                property var tflLine : cppGlue.getTFLLine(lineId)
                columns: 2
                width: parent.width

                Rectangle {
                    implicitWidth: parent.width/2
                    implicitHeight: Math.max(lbl.height, col.height)
                    color: tflLine.Color

                    Label {
                        id: lbl
                        text : modelData["name"]
                        color: "white"
                        wrapMode: Text.Wrap
                        width: parent.width
                        height: implicitHeight*2
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font {
                            pixelSize: 20
                        }
                    }
                    border.width: 1
                    border.color: "black"
                    Layout.topMargin: 10
                }

                Column {
                    id: col
                    width: parent.width/2
                    Layout.fillWidth: true

                    QtObject {
                        id: cache
                        property var descriptions :[]
                    }

                    Repeater {
                        model: modelData["lineStatuses"]

                        onItemAdded: (index) => {
                                         var reason = modelData["lineStatuses"][index]["reason"]

                                         if( cache.descriptions.indexOf(reason) ==-1) {
                                             cache.descriptions.push(reason)
                                         } else
                                         item.visible = false
                                     }

                        Column {
                            width: parent.width

                            Label {
                                id: lblStatus
                                text: modelData["statusSeverityDescription"]
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.Wrap
                                width: parent.width
                                visible: reasonLbl.reason === undefined
                                font {
                                    pixelSize: 20
                                }
                            }
                            Row {
                                width: parent.width
                                Button{
                                    id : butShow
                                    text:reasonLbl.visible ? lblStatus.text : (reasonLbl.reason !== undefined ? lblStatus.text  :"")
                                    onClicked: if(reasonLbl.reason !== undefined)
                                                   reasonLbl.visible = !reasonLbl.visible
                                    visible: reasonLbl.reason !== undefined
                                    font {
                                        pixelSize: 20
                                    }
                                }
                                Label {
                                    text:reasonLbl.visible ? "\u2191" : (reasonLbl.reason != null ? "\u2193" :"")
                                    font {
                                        pixelSize: 20
                                    }
                                }
                            }

                            Label {
                                id: reasonLbl
                                property var reason :modelData["reason"]
                                text: reason !== undefined ?reason:""
                                visible: false//reason != null
                                horizontalAlignment: Text.AlignLeft
                                wrapMode: Text.Wrap
                                width: parent.width
                            }
                        }
                    }
                }
            }
        }
    }
}
