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
    ListModel { id: textPosModel }
    ListModel { id: lengthModel }
    ListModel { id: fontSizeModel }

    Component.onCompleted: {
        for( var i = RadarBlipOption.Start +1; i < RadarBlipOption.End; ++i )
            symbolModel.append({title: cppGlue.getBlipOptionString(i), data : i})

        for(i=10; i <100; i+=10)
            scaleModel.append( {title:i/100.0, data: i})

        for( i=100; i <= 400; i+=25)
            scaleModel.append( {title:i/100.0, data: i})

        for(i=10; i <100; i+=10)
            widthModel.append( {title:i/100.0, data: i})

        for( i=100; i <= 400; i+=25)
            widthModel.append( {title:i/100.0, data: i})

        for( i = RadarTextPositionOption.Start+1; i < RadarTextPositionOption.End; ++i)
            textPosModel.append( { title: cppGlue.getBlipTextPosString(i), data : i })

        for(i=100; i <= 1000; i+=50)
            lengthModel.append( {title:i/100.0, data: i})

        for(i = 4; i < 60; i++)
            fontSizeModel.append( {title: i, data: i})
    }

    function onUpdate( visible) {

        if( visible) {

            stick.checked = radarProfile.blipDrawStick
            symbolCombo.currentIndex = radarProfile.blip-1
            scaleCombo.currentIndex = Helper.findIndex(scaleModel, radarProfile.blipScale)
            widthCombo.currentIndex = Helper.findIndex(widthModel, radarProfile.blipWidth)
            textPosCombo.currentIndex = Helper.findIndex(textPosModel, radarProfile.textPosition)
            lengthCombo.currentIndex = Helper.findIndex(lengthModel, radarProfile.blipStickLength)
            fontFamilyCombo.currentIndex = fontFamilyCombo.find(radarProfile.blipFont)
            fontSizeCombo.currentIndex = Helper.findIndex(fontSizeModel, radarProfile.blipFontSize)

            selectedColor.color = radarProfile.selectedBlipColor
            foreTextColor.color = radarProfile.blipBackTextColor
            backTextColor.color = radarProfile.blipTextColor
            lowTextColor.color = radarProfile.blipFromTextColor
            highTextColor.color = radarProfile.blipToTextColor
            lowAltColor.color = radarProfile.blipFromColor
            highAltColor.color = radarProfile.blipToColor
            climbColor.color = radarProfile.blipUpColor
            descendColor.color = radarProfile.blipDownColor

        }
        else {
            radarProfile.blip = symbolModel.get(symbolCombo.currentIndex).data
            radarProfile.blipDrawStick = stick.checked
            radarProfile.blipScale = scaleModel.get(scaleCombo.currentIndex).data
            radarProfile.blipWidth = widthModel.get(widthCombo.currentIndex).data
            radarProfile.textPosition = textPosModel.get(textPosCombo.currentIndex).data
            radarProfile.blipStickLength = lengthModel.get(lengthCombo.currentIndex).data
            radarProfile.blipFont = fontFamilyCombo.currentText
            radarProfile.blipFontSize = fontSizeModel.get(fontSizeCombo.currentIndex).data

            radarProfile.selectedBlipColor = selectedColor.color
            radarProfile.blipBackTextColor = foreTextColor.color
            radarProfile.blipTextColor = backTextColor.color
            radarProfile.blipFromTextColor = lowTextColor.color
            radarProfile.blipToTextColor = highTextColor.color
            radarProfile.blipFromColor = lowAltColor.color
            radarProfile.blipToColor = highAltColor.color
            radarProfile.blipUpColor = climbColor.color
            radarProfile.blipDownColor = descendColor.color

        }
    }

    GridLayout {
        columns: 2
        width: parent.width*0.9

        SwitchDelegate {
            id: stick
            text: "Stick"
            Layout.columnSpan: 2
            Layout.fillWidth: true
        }

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

        Label { text: "Scale:"; Layout.fillWidth: true }

        ComboBox {
            id: scaleCombo
            model: scaleModel
            textRole: "title"
        }

        Label { text: "Width:"; Layout.fillWidth: true }

        ComboBox {
            id: widthCombo
            model: widthModel
            textRole: "title"
        }

        Label { text: "Position:"; Layout.fillWidth: true  }

        ComboBox {
            id: textPosCombo
            model: textPosModel
            textRole: "title"
        }

        Label { text: "Length:"; Layout.fillWidth: true  }

        ComboBox {
            id: lengthCombo
            model: lengthModel
            textRole: "title"
        }

        Label { text: "Font Family:"; Layout.fillWidth: true  }

        ComboBox {
            id: fontFamilyCombo
            model: Qt.fontFamilies()
        }

        Label { text: "Font Size:"; Layout.fillWidth: true  }

        ComboBox {
            id: fontSizeCombo
            model: fontSizeModel
            textRole: "title"
        }

        Label { text: "Selected:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: selectedColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Back Text (Symbol 'Letter'):"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: backTextColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Fore Text (Symbol 'Letter'):"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: foreTextColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Low Alt Text:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: lowTextColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "High Alt Text:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: highTextColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Low Alt Blip:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: lowAltColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "High Alt Blip:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: highAltColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Climbing:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: climbColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Descending:"; Layout.fillWidth: true  }

        ColorPickerRectangle {
            id: descendColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }
    }
}
