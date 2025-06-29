import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../"
import CppClassLib
import "Helper.js" as Helper

Column {
    id : col

    ListModel { id: symbolModel }
    ListModel { id: scaleModel }
    ListModel { id: widthModel }
    ListModel { id: lengthModel }
    ListModel { id: intervalModel }

    Component.onCompleted: {
        for( var i = RadarTrailOption.Start +1; i < RadarTrailOption.End; ++i )
            symbolModel.append({title: cppGlue.getTrailOptionString(i), data : i})

        for( i=10; i <100; i+=10)
            scaleModel.append({ title:i/100.0, data :i})

        for( i=100; i <= 400; i+=25)
            scaleModel.append({ title:i/100.0, data :i})

        for( i=10; i <100; i+=10)
            widthModel.append({ title:i/100.0, data :i})

        for( i=100; i <= 400; i+=25)
            widthModel.append({ title:i/100.0, data :i})

        for(i=0; i <= 30; ++i)
            lengthModel.append({ title:i, data :i})

        for(i=1; i <= 30; ++i)
            intervalModel.append({ title:i, data :i})
    }

    function onUpdate( visible) {

        if( visible) {
            gettingSmaller.checked = radarProfile.gettingSmaller
            gettingDimmer.checked = radarProfile.gettingDimmer

            symbolCombo.currentIndex = radarProfile.trail-1
            scaleCombo.currentIndex = Helper.findIndex(scaleModel, radarProfile.trailScale)
            widthCombo.currentIndex = Helper.findIndex(widthModel, radarProfile.trailWidth)
            lengthCombo.currentIndex = Helper.findIndex(lengthModel, radarProfile.trailLength)

            intervalCombo.currentIndex = Helper.findIndex(intervalModel, radarProfile.trailIntervalSec)

            colorRect.color = radarProfile.trailColor

        }
        else {
            radarProfile.gettingSmaller = gettingSmaller.checked
            radarProfile.gettingDimmer = gettingDimmer.checked

            radarProfile.trail = symbolModel.get(symbolCombo.currentIndex).data
            radarProfile.trailScale = scaleModel.get(scaleCombo.currentIndex).data
            radarProfile.trailWidth = widthModel.get(widthCombo.currentIndex).data
            radarProfile.trailLength = lengthModel.get(lengthCombo.currentIndex).data

            radarProfile.trailIntervalSec = intervalModel.get(intervalCombo.currentIndex).data

            radarProfile.trailColor = colorRect.color
        }
    }

    GridLayout {
        columns: 2
        width: parent.width*0.9

        SwitchDelegate {
            id: gettingSmaller
            text: "Smaller"
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        SwitchDelegate {
            id: gettingDimmer
            text: "Dimming"
            width: parent.width
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

        Label {
            text: "Symbol:"
            Layout.preferredWidth: parent.width/3
            Layout.fillWidth: true
        }

        ComboBox {
            id: symbolCombo
            model: symbolModel
            textRole: "title"
            Layout.fillWidth: true
        }

        Label { text: "Scale:" }

        ComboBox {
            id: scaleCombo
            model: scaleModel
            textRole: "title"
        }

        Label { text: "Width:" }

        ComboBox {
            id: widthCombo
            model: widthModel
            textRole: "title"
        }

        Label { text: "Length:" }

        ComboBox {
            id: lengthCombo
            model: lengthModel
            textRole: "title"
        }

        Label { text: "Interval:" }

        ComboBox {
            id: intervalCombo
            model: intervalModel
            textRole: "title"
        }

        Label { text: "Color:"}

        ColorPickerRectangle {
            id: colorRect
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }
    }
}
