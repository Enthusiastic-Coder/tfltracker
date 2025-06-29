import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../"
import CppClassLib
import "Helper.js" as Helper

Column {
    id : col

    ListModel { id: symbolModel }
    ListModel { id: minutesModel }
    ListModel { id: widthModel }

    Component.onCompleted: {
        for( var i = RadarFutureOption.Start +1; i < RadarFutureOption.End; ++i )
            symbolModel.append({title: cppGlue.getRadarFutureOptionString(i), data : i})

        for(i=50; i < 500; i+=50)
            minutesModel.append( {title: i/100.0, data: i})

        for(i=500; i <= 2000; i+=100)
            minutesModel.append( {title: i/100.0, data: i})


        for(i=10; i <100; i+=10)
            widthModel.append( { title: i/100.0, data: i} );

        for( i=100; i <= 400; i+=25)
            widthModel.append( { title: i/100.0, data: i} );
    }

    function onUpdate( visible) {

        if( visible) {

            symbolCombo.currentIndex = radarProfile.future-1
            minutesCombo.currentIndex = Helper.findIndex(minutesModel, radarProfile.futureTimeLength)
            widthCombo.currentIndex = Helper.findIndex(widthModel, radarProfile.futureWidth)

            lineColor.color = radarProfile.futureColor

        }
        else {
            radarProfile.future = symbolModel.get(symbolCombo.currentIndex).data
            radarProfile.futureTimeLength = minutesModel.get(minutesCombo.currentIndex).data
            radarProfile.futureWidth = widthModel.get(widthCombo.currentIndex).data

            radarProfile.futureColor = lineColor.color
        }
    }

    GridLayout {
        columns: 2
        width: parent.width*0.9

        Label {
            text: "Symbol:"
            Layout.fillWidth: true
        }

        ComboBox {
            id: symbolCombo
            model: symbolModel
            textRole: "title"
            Layout.fillWidth: true
        }

        Label { text: "Minutes:" }

        ComboBox {
            id: minutesCombo
            model: minutesModel
            textRole: "title"
            Layout.fillWidth: true
        }

        Label { text: "Width:" }

        ComboBox {
            id: widthCombo
            model: widthModel
            textRole: "title"
            Layout.fillWidth: true
        }

        Label { text: "Color:" }

        ColorPickerRectangle {
            id: lineColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }
    }
}
