import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../"
import CppClassLib
import "Helper.js" as Helper

Column {
    id : col

    function onUpdate( visible) {

        if( visible) {

            landColor.color = radarProfile.radarLandColor
            seaColor.color= radarProfile.radarSeaColor
            coastLineColor.color = radarProfile.coastLineColor
            sectorsColor.color =  radarProfile.sectorColor
            airspaceColor.color = radarProfile.airspaceColor
            tmaColor.color = radarProfile.airspaceTMAColor
            fillColor.color = radarProfile.airspaceFillColor
            upperColor.color = radarProfile.airspaceUpperColor
            militaryColor.color = radarProfile.militaryColor
            beaconColor.color = radarProfile.radarBeaconColor
            localiserColor.color= radarProfile.radarRunwayLocaliserColor
            perimeterColor.color =  radarProfile.radarRunwayPerimeterColor
            runwayNumbersColor.color = radarProfile.radarRunwayNumbersColor
            airportColor.color = radarProfile.radarAirportColor
            placesColor.color = radarProfile.radarPlacesColor
            citiesColor.color = radarProfile.radarCityColor
            historyColor.color = radarProfile.historyColor
            ringsColor.color = radarProfile.radarRingsColor

        }
        else {

            radarProfile.radarLandColor = landColor.color
            radarProfile.radarSeaColor = seaColor.color
            radarProfile.coastLineColor = coastLineColor.color
            radarProfile.sectorColor = sectorsColor.color
            radarProfile.airspaceColor = airspaceColor.color
            radarProfile.airspaceTMAColor = tmaColor.color
            radarProfile.airspaceFillColor = fillColor.color
            radarProfile.airspaceUpperColor = upperColor.color
            radarProfile.militaryColor = militaryColor.color
            radarProfile.radarBeaconColor = beaconColor.color
            radarProfile.radarRunwayLocaliserColor = localiserColor.color
            radarProfile.radarRunwayPerimeterColor = perimeterColor.color
            radarProfile.radarRunwayNumbersColor = runwayNumbersColor.color
            radarProfile.radarAirportColor = airportColor.color
            radarProfile.radarPlacesColor = placesColor.color
            radarProfile.radarCityColor = citiesColor.color
            radarProfile.historyColor = historyColor.color
            radarProfile.radarRingsColor = ringsColor.color


        }
    }

    GridLayout {
        columns: 2
        width: parent.width*0.9


        Label { text: "Land Color:" }

        ColorPickerRectangle {
            id: landColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Sea Color:" }

        ColorPickerRectangle {
            id: seaColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "CoastLine:" }

        ColorPickerRectangle {
            id: coastLineColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Sectors:" }

        ColorPickerRectangle {
            id: sectorsColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Airspace Airport:" }

        ColorPickerRectangle {
            id: airspaceColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Airspace TMA:" }

        ColorPickerRectangle {
            id: tmaColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Airspace Fill:" }

        ColorPickerRectangle {
            id: fillColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Upper:" }

        ColorPickerRectangle {
            id: upperColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Military:" }

        ColorPickerRectangle {
            id: militaryColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Beacon:" }

        ColorPickerRectangle {
            id: beaconColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Localiser:" }

        ColorPickerRectangle {
            id: localiserColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Perimeter:" }

        ColorPickerRectangle {
            id: perimeterColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Runway Numbers:" }

        ColorPickerRectangle {
            id: runwayNumbersColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Airport color:" }

        ColorPickerRectangle {
            id: airportColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Places:" }

        ColorPickerRectangle {
            id: placesColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Cities:" }

        ColorPickerRectangle {
            id: citiesColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "History:" }

        ColorPickerRectangle {
            id: historyColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }

        Label { text: "Rings:" }

        ColorPickerRectangle {
            id: ringsColor
            Layout.fillWidth: true
            Layout.preferredHeight: 40
            border.width: 1
        }
    }
}
