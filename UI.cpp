#include "UI.h"

#include <QtCore/QVariant>
#include "Units.h"
#include "TrackerGlue.h"
#include "TFLViewFrameBuffer.h"

Ui_QtAtcXClass::Ui_QtAtcXClass(QObject *parent) : QObject(parent)
{
}

void Ui_QtAtcXClass::trigger(const QString &objectName)
{
    Action* a = QObject::findChild<Action*>(objectName);
    if( a != nullptr)
        a->trigger();
    else
        qDebug() << objectName << " NOT FOUND";
}

void Ui_QtAtcXClass::toggle(const QString &objectName, bool value)
{
    Action* a = QObject::findChild<Action*>(objectName);
    if( a != nullptr)
        a->setChecked(value);
    else
        qDebug() << objectName << " NOT FOUND";
}

void Ui_QtAtcXClass::setupUi()
{
    if (parent()->objectName().isEmpty())
        parent()->setObjectName(QStringLiteral("CPPGlue"));

    action_Exit = new Action(this);
    action_Exit->setObjectName(QStringLiteral("action_Exit"));
    action_ZoomIn = new Action(this);
    action_ZoomIn->setObjectName(QStringLiteral("action_ZoomIn"));
    action_ZoomOut = new Action(this);
    action_ZoomOut->setObjectName(QStringLiteral("action_ZoomOut"));
    actionShow = new Action(this);
    actionShow->setObjectName(QStringLiteral("actionShow"));

    action_InAppCheckMe = new Action(this);
    action_InAppCheckMe->setObjectName(QStringLiteral("action_InAppCheckMe"));

    action_Close_Simulation = new Action(this);
    action_Close_Simulation->setObjectName(QStringLiteral("action_Close_Simulation"));
    action_Close_Simulation->setEnabled(false);
    action_Debug = new Action(this);
    action_Debug->setObjectName(QStringLiteral("action_Debug"));
    action_Debug->setCheckable(true);
    action_Clock = new Action(this);
    action_Clock->setObjectName(QStringLiteral("action_Clock"));
    action_Clock->setCheckable(true);

    action_Pause = new Action(this);
    action_Pause->setObjectName(QStringLiteral("action_Pause"));
    action_Pause->setCheckable(true);
    action_About_Data_Source = new Action(this);
    action_About_Data_Source->setObjectName(QStringLiteral("action_About_Data_Source"));
    action_About_Legal = new Action(this);
    action_About_Legal->setObjectName(QStringLiteral("action_About_Legal"));

    action_Compass = new Action(this);
    action_Compass->setObjectName(QStringLiteral("action_Compass"));
    action_Compass->setCheckable(true);

    action_GPS = new Action(this);
    action_GPS->setObjectName(QStringLiteral("action_GPS"));

    action_Call_Sign = new Action(this);
    action_Call_Sign->setObjectName(QStringLiteral("action_Call_Sign"));
    action_Call_Sign->setCheckable(true);

    action_Places = new Action(this);
    action_Places->setObjectName(QStringLiteral("action_Places"));
    action_Places->setCheckable(true);

    action_RealTime_GPS = new Action(this);
    action_RealTime_GPS->setObjectName(QStringLiteral("action_RealTime_GPS"));
    action_RealTime_GPS->setCheckable(true);

    action_InApp_ProximityAlert = new Action(this);
    action_InApp_ProximityAlert->setObjectName(QStringLiteral("action_InApp_ProximityAlert"));
    action_InApp_ProximityAlert->setCheckable(false);
    action_InApp_ProximityAlert->setEnabled(true);

    action_Purchase_Monthly_Sub = new Action(this);
    action_Purchase_Monthly_Sub->setObjectName(QStringLiteral("action_Purchase_TFLTrackerSubscription"));
    action_Purchase_Monthly_Sub->setCheckable(false);
    action_Purchase_Monthly_Sub->setEnabled(true);

    action_Purchase_Lifetime = new Action(this);
    action_Purchase_Lifetime->setObjectName(QStringLiteral("action_Purchase_Lifetime"));
    action_Purchase_Lifetime->setCheckable(false);
    action_Purchase_Lifetime->setEnabled(true);

    action_InApp_VehiclePosition = new Action(this);
    action_InApp_VehiclePosition->setObjectName(QStringLiteral("action_InApp_VehiclePosition"));
    action_InApp_VehiclePosition->setCheckable(false);
    action_InApp_VehiclePosition->setEnabled(true);

    action_Release_Notes = new Action(this);
    action_Release_Notes->setObjectName(QStringLiteral("action_Release_Notes"));

    action_Bus_Verbosity_None = new Action(this);
    action_Bus_Verbosity_None->setObjectName(QStringLiteral("action_Bus_Verbosity_None"));
    action_Bus_Verbosity_None->setCheckable(true);

    action_Bus_Verbosity_LineId = new Action(this);
    action_Bus_Verbosity_LineId->setObjectName(QStringLiteral("action_Bus_Verbosity_LineId"));
    action_Bus_Verbosity_LineId->setCheckable(true);

    action_Bus_Verbosity_VehicleId = new Action(this);
    action_Bus_Verbosity_VehicleId->setObjectName(QStringLiteral("action_Bus_Verbosity_VehicleId"));
    action_Bus_Verbosity_VehicleId->setCheckable(true);

    action_Bus_Verbosity_All = new Action(this);
    action_Bus_Verbosity_All->setObjectName(QStringLiteral("action_Bus_Verbosity_All"));
    action_Bus_Verbosity_All->setCheckable(true);

//////////////////////
    action_Train_Verbosity_None = new Action(this);
    action_Train_Verbosity_None->setObjectName(QStringLiteral("action_Train_Verbosity_None"));
    action_Train_Verbosity_None->setCheckable(true);

    action_Train_Verbosity_LineId = new Action(this);
    action_Train_Verbosity_LineId->setObjectName(QStringLiteral("action_Train_Verbosity_LineId"));
    action_Train_Verbosity_LineId->setCheckable(true);

    action_Train_Verbosity_VehicleId = new Action(this);
    action_Train_Verbosity_VehicleId->setObjectName(QStringLiteral("action_Train_Verbosity_VehicleId"));
    action_Train_Verbosity_VehicleId->setCheckable(true);

    action_Train_Verbosity_All = new Action(this);
    action_Train_Verbosity_All->setObjectName(QStringLiteral("action_Train_Verbosity_All"));
    action_Train_Verbosity_All->setCheckable(true);
//////////////////////
    action_Proximity_Active = new Action(this);
    action_Proximity_Active->setObjectName(QStringLiteral("action_Proximity_Active"));
    action_Proximity_Active->setCheckable(true);

    action_London_Information = new Action(this);
    action_London_Information->setObjectName(QStringLiteral("action_London_Information"));
    action_London_Information->setCheckable(true);

    action_2D_Map_Night = new Action(this);
    action_2D_Map_Night->setObjectName(QStringLiteral("action_2D_Map_Night"));
    action_2D_Map_Night->setCheckable(true);

    action_OSM_Motorway_Visible = new Action(this);
    action_OSM_Motorway_Visible->setObjectName(QStringLiteral("action_OSM_Motorway_Visible"));
    action_OSM_Motorway_Visible->setText("Motorway");
    action_OSM_Motorway_Visible->setCheckable(true);

    action_OSM_Primary_Visible = new Action(this);
    action_OSM_Primary_Visible->setObjectName(QStringLiteral("action_OSM_Primary_Visible"));
    action_OSM_Primary_Visible->setText("Primary");
    action_OSM_Primary_Visible->setCheckable(true);

    action_OSM_Secondary_Visible = new Action(this);
    action_OSM_Secondary_Visible->setObjectName(QStringLiteral("action_OSM_Secondary_Visible"));
    action_OSM_Secondary_Visible->setText("Secondary");
    action_OSM_Secondary_Visible->setCheckable(true);

    action_OSM_Tertiary_Visible = new Action(this);
    action_OSM_Tertiary_Visible->setObjectName(QStringLiteral("action_OSM_Tertiary_Visible"));
    action_OSM_Tertiary_Visible->setText("Tertiary");
    action_OSM_Tertiary_Visible->setCheckable(true);

    action_OSM_Residential_Visible = new Action(this);
    action_OSM_Residential_Visible->setObjectName(QStringLiteral("action_OSM_Residential_Visible"));
    action_OSM_Residential_Visible->setText("Residential");
    action_OSM_Residential_Visible->setCheckable(true);

    action_OSM_Footway_Visible = new Action(this);
    action_OSM_Footway_Visible->setObjectName(QStringLiteral("action_OSM_Footway_Visible"));
    action_OSM_Footway_Visible->setText("Footway");
    action_OSM_Footway_Visible->setCheckable(true);

    action_OSM_Water_Visible = new Action(this);
    action_OSM_Water_Visible->setObjectName(QStringLiteral("action_OSM_Water_Visible"));
    action_OSM_Water_Visible->setText("Water");
    action_OSM_Water_Visible->setCheckable(true);

    action_OSM_Aeroway_Visible = new Action(this);
    action_OSM_Aeroway_Visible->setObjectName(QStringLiteral("action_OSM_Aeroway_Visible"));
    action_OSM_Aeroway_Visible->setText("Airports");
    action_OSM_Aeroway_Visible->setCheckable(true);

    action_OSM_CycleWay_Visible = new Action(this);
    action_OSM_CycleWay_Visible->setObjectName(QStringLiteral("action_OSM_CycleWay_Visible"));
    action_OSM_CycleWay_Visible->setText("CycleWay");
    action_OSM_CycleWay_Visible->setCheckable(true);

    action_OSM_Pedestrian_Visible = new Action(this);
    action_OSM_Pedestrian_Visible->setObjectName(QStringLiteral("action_OSM_Pedestrian_Visible"));
    action_OSM_Pedestrian_Visible->setText("Pedestrian");
    action_OSM_Pedestrian_Visible->setCheckable(true);

    action_TFL_BusStop_Visible = new Action(this);
    action_TFL_BusStop_Visible->setObjectName(QStringLiteral("action_TFL_BusStop_Visible"));
    action_TFL_BusStop_Visible->setText("Bus Stops");
    action_TFL_BusStop_Visible->setCheckable(true);

    action_TFL_BusLine_Visible = new Action(this);
    action_TFL_BusLine_Visible->setObjectName(QStringLiteral("action_TFL_BusLine_Visible"));
    action_TFL_BusLine_Visible->setText("Bus Lines");
    action_TFL_BusLine_Visible->setCheckable(true);

    action_TFL_TubeLine_Visible = new Action(this);
    action_TFL_TubeLine_Visible->setObjectName(QStringLiteral("action_TFL_TubeLine_Visible"));
    action_TFL_TubeLine_Visible->setText("Tube Lines");
    action_TFL_TubeLine_Visible->setCheckable(true);

    action_piccadillyNormalDestination = new Action(this);
    action_piccadillyNormalDestination->setObjectName(QStringLiteral("piccadillyDestination"));
    action_piccadillyNormalDestination->setText("Piccadilly Destination Show 'Heathrow' before Terminals");
    action_piccadillyNormalDestination->setCheckable(true);

    action_Use_Vehicle_Behaviour = new Action(this);
    action_Use_Vehicle_Behaviour->setObjectName(QStringLiteral("action_Use_Vehicle_Behaviour"));
    action_Use_Vehicle_Behaviour->setText(QStringLiteral("Use 'Vehicle Location' to guess position"));
    action_Use_Vehicle_Behaviour->setCheckable(true);

    action_Elizabeth_Arrivals_Use_TFL_Data = new Action(this);
    action_Elizabeth_Arrivals_Use_TFL_Data->setObjectName(QStringLiteral("action_Elizabeth_Arrivals_Use_TFL_Data"));
    action_Elizabeth_Arrivals_Use_TFL_Data->setText(QStringLiteral("Elizabeth Line train position from TFL data (not Network Rail)"));
    action_Elizabeth_Arrivals_Use_TFL_Data->setCheckable(true);

    action_Proximity_Mute_sound = new Action(this);
    action_Proximity_Mute_sound->setObjectName(QStringLiteral("action_Proximity_Mute_sound"));
    action_Proximity_Mute_sound->setCheckable(true);

    action_Show_Proximity_Rings = new Action(this);
    action_Show_Proximity_Rings->setObjectName(QStringLiteral("action_Show_Proximity_Rings"));
    action_Show_Proximity_Rings->setCheckable(true);
    action_Show_Proximity_Rings->setChecked(false);

    action_Instruction_Notes = new Action(this);
    action_Instruction_Notes->setObjectName(QStringLiteral("action_Instruction_Notes"));
    action_Display_Short_Destination = new Action(this);
    action_Display_Short_Destination->setObjectName(QStringLiteral("action_Display_Short_Destination"));
    action_Display_Short_Destination->setCheckable(true);

    actionPurchase = new Action(this);
    actionPurchase->setObjectName(QStringLiteral("actionPurchase"));
    actionIsPurchased = new Action(this);
    actionIsPurchased->setObjectName(QStringLiteral("actionIsPurchased"));
    action_android_test_purchased = new Action(this);
    action_android_test_purchased->setObjectName(QStringLiteral("action_android_test_purchased"));
    action_android_test_canceled = new Action(this);
    action_android_test_canceled->setObjectName(QStringLiteral("action_android_test_canceled"));
    action_android_test_refunded = new Action(this);
    action_android_test_refunded->setObjectName(QStringLiteral("action_android_test_refunded"));
    action_android_test_item_unavailable = new Action(this);
    action_android_test_item_unavailable->setObjectName(QStringLiteral("action_android_test_item_unavailable"));
    action_3D_VR = new Action(this);
    action_3D_VR->setObjectName(QStringLiteral("action_3D_VR"));
    action_3D_VR->setCheckable(false);

    action_3D_Blip_Verbosity_None = new Action(this);
    action_3D_Blip_Verbosity_None->setObjectName(QStringLiteral("action_3D_Blip_Verbosity_None"));
    action_3D_Blip_Verbosity_None->setCheckable(true);
    action_3D_Blip_Verbosity_ID = new Action(this);
    action_3D_Blip_Verbosity_ID->setObjectName(QStringLiteral("action_3D_Blip_Verbosity_ID"));
    action_3D_Blip_Verbosity_ID->setCheckable(true);
    action_3D_Blip_Verbosity_All = new Action(this);
    action_3D_Blip_Verbosity_All->setObjectName(QStringLiteral("action_3D_Blip_Verbosity_All"));
    action_3D_Blip_Verbosity_All->setCheckable(true);
    action_3D_Blip_Verbosity_All->setChecked(false);
    action_Units_Spd_Knots = new Action(this);
    action_Units_Spd_Knots->setObjectName(QStringLiteral("action_Units_Spd_Knots"));
    action_Units_Spd_Knots->setCheckable(true);
    action_Units_Spd_Km_H = new Action(this);
    action_Units_Spd_Km_H->setObjectName(QStringLiteral("action_Units_Spd_Km_H"));
    action_Units_Spd_Km_H->setCheckable(true);
    action_Units_Spd_MPH = new Action(this);
    action_Units_Spd_MPH->setObjectName(QStringLiteral("action_Units_Spd_MPH"));
    action_Units_Spd_MPH->setCheckable(true);

    action_Units_Alt_Feet = new Action(this);
    action_Units_Alt_Feet->setObjectName(QStringLiteral("action_Units_Alt_Feet"));
    action_Units_Alt_Feet->setCheckable(true);
    action_Units_Alt_Meters = new Action(this);
    action_Units_Alt_Meters->setObjectName(QStringLiteral("action_Units_Alt_Meters"));
    action_Units_Alt_Meters->setCheckable(true);

    action_Units_Dist_SM = new Action(this);
    action_Units_Dist_SM->setObjectName(QStringLiteral("action_Units_Dist_SM"));
    action_Units_Dist_SM->setCheckable(true);
    action_Units_Dist_KM = new Action(this);
    action_Units_Dist_KM->setObjectName(QStringLiteral("action_Units_Dist_KM"));
    action_Units_Dist_KM->setCheckable(true);
    action_Units_Dist_Meters = new Action(this);
    action_Units_Dist_Meters->setObjectName(QStringLiteral("action_Units_Dist_Meters"));
    action_Units_Dist_Meters->setCheckable(true);

    action_Units_Spd_Meters_Second = new Action(this);
    action_Units_Spd_Meters_Second->setObjectName(QStringLiteral("action_Units_Spd_Meters_Second"));
    action_Units_Spd_Meters_Second->setCheckable(true);

    action_Units_Alt_Nautical_Miles = new Action(this);
    action_Units_Alt_Nautical_Miles->setObjectName(QStringLiteral("action_Units_Alt_Nautical_Miles"));
    action_Units_Alt_Nautical_Miles->setCheckable(true);
    action_Units_Alt_Statue_Miles = new Action(this);
    action_Units_Alt_Statue_Miles->setObjectName(QStringLiteral("action_Units_Alt_Statue_Miles"));
    action_Units_Alt_Statue_Miles->setCheckable(true);
    action_Units_Alt_Kilometers = new Action(this);
    action_Units_Alt_Kilometers->setObjectName(QStringLiteral("action_Units_Alt_Kilometers"));
    action_Units_Alt_Kilometers->setCheckable(true);

    action_InApp_3DView = new Action(this);
    action_InApp_3DView->setObjectName(QStringLiteral("action_InApp_3DView"));
    action_InApp_3DView->setCheckable(false);
    action_InApp_3DView->setEnabled(true);
    action_InApp_RealTimeGPS = new Action(this);
    action_InApp_RealTimeGPS->setObjectName(QStringLiteral("action_InApp_RealTimeGPS"));
    action_InApp_RealTimeGPS->setCheckable(false);
    action_InApp_RealTimeGPS->setEnabled(true);

    action_Units_VSI_Minute = new Action(this);
    action_Units_VSI_Minute->setObjectName(QStringLiteral("action_Units_VSI_Minute"));
    action_Units_VSI_Minute->setCheckable(true);
    action_Units_VSI_Second = new Action(this);
    action_Units_VSI_Second->setObjectName(QStringLiteral("action_Units_VSI_Second"));
    action_Units_VSI_Second->setCheckable(true);

    action_3D_Show_You = new Action(this);
    action_3D_Show_You->setObjectName(QStringLiteral("action_3D_Show_You"));
    action_3D_Show_You->setCheckable(true);
    action_3D_Show_You->setChecked(true);

    menuBar = new Menu(this);
    menuBar->setObjectName(QStringLiteral("menuBar"));
    menu_File = new Menu(menuBar);
    menu_File->setObjectName(QStringLiteral("menu_File"));
    menu_Mode = new Menu(menuBar);
    menu_Mode->setObjectName(QStringLiteral("menu_Mode"));
    menu_View = new Menu(menuBar);
    menu_View->setObjectName(QStringLiteral("menu_View"));
    menuBlip = new Menu(menu_View);
    menuBlip->setObjectName(QStringLiteral("menuBlip"));
    menu_About = new Menu(menuBar);
    menu_About->setObjectName(QStringLiteral("menu_About"));

    menu_Boundaries = new Menu(menuBar);
    menu_Boundaries->setObjectName(QStringLiteral("menu_Boundaries"));

    menu_Labels_Filter = new Menu(menuBar);
    menu_Labels_Filter->setObjectName(QStringLiteral("menu_Labels_Filter"));

    menu_Circle_ID_Color_Override = new Menu(menuBar);
    menu_Circle_ID_Color_Override->setObjectName(QStringLiteral("menu_Circle_ID_Color_Override"));

    menu_TFL_Filter = new Menu(menuBar);
    menu_TFL_Filter->setObjectName(QStringLiteral("menu_TFL_Filter"));

    menu_Settings_General = new Menu(menuBar);
    menu_Settings_General->setObjectName(QStringLiteral("menu_Settings_General"));

    menu_2D = new Menu(menuBar);
    menu_2D->setObjectName(QStringLiteral("menu_2D"));
    menu_3D = new Menu(menuBar);
    menu_3D->setObjectName(QStringLiteral("menu_3D"));
    menuBlip_Verbosity_3D = new Menu(menu_3D);
    menuBlip_Verbosity_3D->setObjectName(QStringLiteral("menuBlip_Verbosity_3D"));
    menuUnits = new Menu(menuBar);
    menuUnits->setObjectName(QStringLiteral("menuUnits"));
    menuSpeed = new Menu(menuUnits);
    menuSpeed->setObjectName(QStringLiteral("menuSpeed"));
    menuAltitude = new Menu(menuUnits);
    menuAltitude->setObjectName(QStringLiteral("menuAltitude"));
    menuDistance = new Menu(menuUnits);
    menuDistance->setObjectName(QStringLiteral("menuDistance"));
    menuVertical_Speed = new Menu(menuUnits);
    menuVertical_Speed->setObjectName(QStringLiteral("menuVertical_Speed"));
    menu_InAppPurchases = new Menu(menuBar);
    menu_InAppPurchases->setObjectName(QStringLiteral("menu_InAppPurchases"));
    menu_Jump_To = new Menu(menuBar);
    menu_Jump_To->setObjectName(QStringLiteral("menu_Jump_To"));

    menuBar->addAction(menu_File->menuAction());
    menuBar->addAction(menu_InAppPurchases->menuAction());
    menuBar->addAction(menu_Jump_To->menuAction());
    menuBar->addAction(menu_Mode->menuAction());
    menuBar->addAction(menu_Labels_Filter->menuAction());
    menuBar->addAction(menuUnits->menuAction());
    menuBar->addAction(menu_2D->menuAction());
    menuBar->addAction(menu_3D->menuAction());
    menuBar->addAction(menu_View->menuAction());
    menuBar->addAction(menu_Boundaries->menuAction());
    menuBar->addAction(menu_About->menuAction());

    menu_File->addAction(action_Close_Simulation);
    menu_File->addAction(action_Pause);
    menu_File->addAction(actionPurchase);
    menu_File->addAction(actionIsPurchased);
    menu_File->addAction(action_android_test_purchased);
    menu_File->addAction(action_android_test_canceled);
    menu_File->addAction(action_android_test_refunded);
    menu_File->addAction(action_android_test_item_unavailable);
    menu_File->addAction(action_Exit);

    //    menu_View->addAction(menuBlip->menuAction());
//    menu_View->addAction(action_Aircraft_List);
//    menu_View->addAction(action_RT);
//    menu_View->addAction(action_Debug);

    menu_View->addAction(action_Clock);
    menu_View->addAction(action_Display_Short_Destination);
    menuBlip->addAction(action_Call_Sign);

    menu_About->addAction(action_About_Data_Source);
    menu_About->addAction(action_Instruction_Notes);
    menu_About->addAction(action_Release_Notes);
    menu_About->addAction(action_About_Legal);

    //menu_Labels_Filter->addAction(action_Places);
    menu_Labels_Filter->addAction(action_OSM_Motorway_Visible);
    menu_Labels_Filter->addAction(action_OSM_Primary_Visible);
    menu_Labels_Filter->addAction(action_OSM_Secondary_Visible);
    menu_Labels_Filter->addAction(action_OSM_Tertiary_Visible);
    menu_Labels_Filter->addAction(action_OSM_Residential_Visible);
    menu_Labels_Filter->addAction(action_OSM_CycleWay_Visible);
    menu_Labels_Filter->addAction(action_OSM_Pedestrian_Visible);
    menu_Labels_Filter->addAction(action_OSM_Footway_Visible);
    menu_Labels_Filter->addAction(action_OSM_Water_Visible);
    menu_Labels_Filter->addAction(action_OSM_Aeroway_Visible);

    menu_TFL_Filter->addAction(action_TFL_BusStop_Visible);
    menu_TFL_Filter->addAction(action_TFL_BusLine_Visible);
    menu_TFL_Filter->addAction(action_TFL_TubeLine_Visible);

    menu_Settings_General->addAction(action_piccadillyNormalDestination);
    menu_Settings_General->addAction(action_Use_Vehicle_Behaviour);
    menu_Settings_General->addAction(action_Elizabeth_Arrivals_Use_TFL_Data);

    menu_3D->addAction(action_3D_Show_You);

    menuBlip_Verbosity_3D->addAction(action_3D_Blip_Verbosity_None);
    menuBlip_Verbosity_3D->addAction(action_3D_Blip_Verbosity_ID);
    menuBlip_Verbosity_3D->addAction(action_3D_Blip_Verbosity_All);

    menuUnits->addAction(menuSpeed->menuAction());
    menuUnits->addAction(menuAltitude->menuAction());
    menuUnits->addAction(menuDistance->menuAction());
    menuUnits->addAction(menuVertical_Speed->menuAction());

    menuSpeed->addAction(action_Units_Spd_Knots);
    menuSpeed->addAction(action_Units_Spd_Km_H);
    menuSpeed->addAction(action_Units_Spd_MPH);
    menuSpeed->addAction(action_Units_Spd_Meters_Second);

    menuAltitude->addAction(action_Units_Alt_Feet);
    menuAltitude->addAction(action_Units_Alt_Meters);
    menuAltitude->addAction(action_Units_Alt_Nautical_Miles);
    menuAltitude->addAction(action_Units_Alt_Statue_Miles);
    menuAltitude->addAction(action_Units_Alt_Kilometers);

    menuDistance->addAction(action_Units_Dist_SM);
    menuDistance->addAction(action_Units_Dist_KM);
    menuDistance->addAction(action_Units_Dist_Meters);

    menuVertical_Speed->addAction(action_Units_VSI_Minute);
    menuVertical_Speed->addAction(action_Units_VSI_Second);

    menu_InAppPurchases->addAction(action_InApp_3DView);
    menu_InAppPurchases->addAction(action_InApp_RealTimeGPS);
    menu_InAppPurchases->addAction(action_InApp_ProximityAlert);
    menu_InAppPurchases->addAction(action_InApp_VehiclePosition);
    menu_InAppPurchases->addAction(action_Purchase_Monthly_Sub);
    menu_InAppPurchases->addAction(action_Purchase_Lifetime);

    menu_Label_Bus_Verbosity = new ActionGroup(this);
    menu_Label_Bus_Verbosity->setTitle("Bus Label Config:");
    menu_Label_Bus_Verbosity->addAction(action_Bus_Verbosity_None);
    menu_Label_Bus_Verbosity->addAction(action_Bus_Verbosity_LineId);
    menu_Label_Bus_Verbosity->addAction(action_Bus_Verbosity_VehicleId);
    menu_Label_Bus_Verbosity->addAction(action_Bus_Verbosity_All);

    menu_Label_Train_Verbosity = new ActionGroup(this);
    menu_Label_Train_Verbosity->setTitle("Train Label Config:");
    menu_Label_Train_Verbosity->addAction(action_Train_Verbosity_None);
    menu_Label_Train_Verbosity->addAction(action_Train_Verbosity_LineId);
    menu_Label_Train_Verbosity->addAction(action_Train_Verbosity_VehicleId);
    menu_Label_Train_Verbosity->addAction(action_Train_Verbosity_All);

    menu3DBlipVerbosity = new ActionGroup(this);
    menu3DBlipVerbosity->addAction(action_3D_Blip_Verbosity_None);
    menu3DBlipVerbosity->addAction(action_3D_Blip_Verbosity_ID);
    menu3DBlipVerbosity->addAction(action_3D_Blip_Verbosity_All);

    menuUnitsSpeed = new ActionGroup(this);
    menuUnitsSpeed->setTitle("Speed");
    menuUnitsSpeed->addAction(action_Units_Spd_Km_H);
    menuUnitsSpeed->addAction(action_Units_Spd_Knots);
    menuUnitsSpeed->addAction(action_Units_Spd_Meters_Second);
    menuUnitsSpeed->addAction(action_Units_Spd_MPH);

    menuUnitsDistance = new ActionGroup(this);
    menuUnitsDistance->setTitle("Distance");
    menuUnitsDistance->addAction(action_Units_Dist_KM);
    menuUnitsDistance->addAction(action_Units_Dist_Meters);
    menuUnitsDistance->addAction(action_Units_Dist_SM);

    menuUnitsAltitude = new ActionGroup(this);
    menuUnitsAltitude->setTitle("Altitude");
    menuUnitsAltitude->addAction(action_Units_Alt_Feet);
    menuUnitsAltitude->addAction(action_Units_Alt_Meters);
    menuUnitsAltitude->addAction(action_Units_Alt_Nautical_Miles);
    menuUnitsAltitude->addAction(action_Units_Alt_Statue_Miles);
    menuUnitsAltitude->addAction(action_Units_Alt_Kilometers);

    menuUnitsVsiInterval = new ActionGroup(this);
    menuUnitsVsiInterval->setTitle("Vertical Speed");
    menuUnitsVsiInterval->addAction(action_Units_VSI_Minute);
    menuUnitsVsiInterval->addAction(action_Units_VSI_Second);

    menu2DRadarProfile = new ActionGroup(this);
    actionGroup_skyLineGroup = new ActionGroup(this);
    actionGroup_skyLineGroup->setTitle("Sky Line");

    lineGroup_coach = new Menu(this);
    lineGroup_coach->setTitle("Coach");

    lineGroup_cycle = new Menu(this);
    lineGroup_cycle->setTitle("Cycle");

    lineGroup_cycle_hire = new Menu(this);
    lineGroup_cycle_hire->setTitle("Cycle Hire");

    lineGroup_replacement_bus = new Menu(this);
    lineGroup_replacement_bus->setTitle("Replacement Bus");

    lineGroup_river_bus = new Menu(this);
    lineGroup_river_bus->setTitle("River Bus");

    lineGroup_river_tour = new Menu(this);
    lineGroup_river_tour->setTitle("River Tour");

    lineGroup_taxi = new Menu(this);
    lineGroup_taxi->setTitle("Taxi");

    lineGroup_tube = new Menu(this);
    lineGroup_tube->setTitle("Tube");

    lineGroup_national_rail = new Menu(this);
    lineGroup_national_rail->setTitle("National Rail");

    lineGroup_tfl_rail = new Menu(this);
    lineGroup_tfl_rail->setTitle("TFL Rail");
    
    lineGroup_cable_car = new Menu(this);
    lineGroup_cable_car->setTitle("Cable Car");

    retranslateUi();

    QMetaObject::connectSlotsByName(this);
}

void Ui_QtAtcXClass::setGPSSource(QGeoPositionInfoSource *p)
{
    _locationInfo = p;
}

 // setupUi

void Ui_QtAtcXClass::retranslateUi()
{
    action_Exit->setText(QCoreApplication::translate("QtAtcXClass", "E&xit", nullptr));
    action_ZoomIn->setText(QCoreApplication::translate("QtAtcXClass", "+", nullptr));
    action_ZoomOut->setText(QCoreApplication::translate("QtAtcXClass", "-", nullptr));
    actionShow->setText(QCoreApplication::translate("QtAtcXClass", "Show", nullptr));

    action_InAppCheckMe->setText(QCoreApplication::translate("QtAtcXClass", "Enforce Evaluation", nullptr));

    action_Close_Simulation->setText(QCoreApplication::translate("QtAtcXClass", "&Close Simulation", nullptr));
    action_Debug->setText(QCoreApplication::translate("QtAtcXClass", "&Debug", nullptr));
    action_Clock->setText(QCoreApplication::translate("QtAtcXClass", "&Clock", nullptr));

    action_Pause->setText(QCoreApplication::translate("QtAtcXClass", "Pause", nullptr));
    action_About_Data_Source->setText(QCoreApplication::translate("QtAtcXClass", "Credits", nullptr));
    action_About_Legal->setText(QCoreApplication::translate("QtAtcXClass", "Disclaimer", nullptr));
    action_Compass->setText(QCoreApplication::translate("QtAtcXClass", "Compass", nullptr));
    action_GPS->setText(QCoreApplication::translate("QtAtcXClass", "Current GPS ", nullptr));
    action_Call_Sign->setText(QCoreApplication::translate("QtAtcXClass", "Call Sign", nullptr));
    action_Places->setText(QCoreApplication::translate("QtAtcXClass", "Places", nullptr));

    action_RealTime_GPS->setText(QCoreApplication::translate("QtAtcXClass", "Start RealTime GPS", nullptr));
    action_Release_Notes->setText(QCoreApplication::translate("QtAtcXClass", "Release Notes", nullptr));

    action_Bus_Verbosity_None->setText(QCoreApplication::translate("QtAtcXClass", "None", nullptr));
    action_Bus_Verbosity_LineId->setText(QCoreApplication::translate("QtAtcXClass", "Line Id", nullptr));
    action_Bus_Verbosity_VehicleId->setText(QCoreApplication::translate("QtAtcXClass", "Vehicle Id", nullptr));
    action_Bus_Verbosity_All->setText(QCoreApplication::translate("QtAtcXClass", "All", nullptr));

    action_Train_Verbosity_None->setText(QCoreApplication::translate("QtAtcXClass", "None", nullptr));
    action_Train_Verbosity_LineId->setText(QCoreApplication::translate("QtAtcXClass", "Line Id", nullptr));
    action_Train_Verbosity_VehicleId->setText(QCoreApplication::translate("QtAtcXClass", "Vehicle Id", nullptr));
    action_Train_Verbosity_All->setText(QCoreApplication::translate("QtAtcXClass", "All", nullptr));

    action_Proximity_Active->setText(QCoreApplication::translate("QtAtcXClass", "Active", nullptr));
    action_London_Information->setText(QCoreApplication::translate("QtAtcXClass", "London Information", nullptr));

    action_Proximity_Mute_sound->setText(QCoreApplication::translate("QtAtcXClass", "Mute sound", nullptr));
    action_Show_Proximity_Rings->setText(QCoreApplication::translate("QtAtcXClass", "Proximity Rings", nullptr));
    action_Instruction_Notes->setText(QCoreApplication::translate("QtAtcXClass", "Instructions", nullptr));
    action_Display_Short_Destination->setText(QCoreApplication::translate("QtAtcXClass", "Short Destn", nullptr));
    actionPurchase->setText(QCoreApplication::translate("QtAtcXClass", "Purchase", nullptr));
    actionIsPurchased->setText(QCoreApplication::translate("QtAtcXClass", "IsPurchased", nullptr));
    action_android_test_purchased->setText(QCoreApplication::translate("QtAtcXClass", "android.test.purchased", nullptr));
    action_android_test_canceled->setText(QCoreApplication::translate("QtAtcXClass", "android.test.canceled", nullptr));
    action_android_test_refunded->setText(QCoreApplication::translate("QtAtcXClass", "android.test.refunded", nullptr));
    action_android_test_item_unavailable->setText(QCoreApplication::translate("QtAtcXClass", "android.test.item_unavailable", nullptr));
    action_3D_VR->setText(QCoreApplication::translate("QtAtcXClass", "VR", nullptr));
    action_3D_Blip_Verbosity_None->setText(QCoreApplication::translate("QtAtcXClass", "None", nullptr));
    action_3D_Blip_Verbosity_ID->setText(QCoreApplication::translate("QtAtcXClass", "ID", nullptr));
    action_3D_Blip_Verbosity_All->setText(QCoreApplication::translate("QtAtcXClass", "All", nullptr));
    action_Units_Spd_Knots->setText(QCoreApplication::translate("QtAtcXClass", "Knots", nullptr));
    action_Units_Spd_Km_H->setText(QCoreApplication::translate("QtAtcXClass", "Km/h", nullptr));
    action_Units_Spd_MPH->setText(QCoreApplication::translate("QtAtcXClass", "Mph", nullptr));
    action_Units_Alt_Feet->setText(QCoreApplication::translate("QtAtcXClass", "Feet", nullptr));
    action_Units_Alt_Meters->setText(QCoreApplication::translate("QtAtcXClass", "Meters", nullptr));
    action_Units_Dist_SM->setText(QCoreApplication::translate("QtAtcXClass", "Miles", nullptr));
    action_Units_Dist_KM->setText(QCoreApplication::translate("QtAtcXClass", "Kilometers", nullptr));
    action_Units_Dist_Meters->setText(QCoreApplication::translate("QtAtcXClass", "Meters", nullptr));
    action_Units_Spd_Meters_Second->setText(QCoreApplication::translate("QtAtcXClass", "Meters/Second", nullptr));

    action_Units_Alt_Nautical_Miles->setText(QCoreApplication::translate("QtAtcXClass", "Nautical Miles", nullptr));
    action_Units_Alt_Statue_Miles->setText(QCoreApplication::translate("QtAtcXClass", "Statue Miles", nullptr));
    action_Units_Alt_Kilometers->setText(QCoreApplication::translate("QtAtcXClass", "Kilometers", nullptr));

    action_InApp_3DView->setText(QCoreApplication::translate("QtAtcXClass", "3D View - (Unknown)", nullptr));
    action_InApp_RealTimeGPS->setText(QCoreApplication::translate("QtAtcXClass", "RealTime GPS - (Unknown)", nullptr));
    action_InApp_ProximityAlert->setText(QCoreApplication::translate("QtAtcXClass", "Proximity Warning - (Unknown)", nullptr));
    action_InApp_VehiclePosition->setText(QCoreApplication::translate("QtAtcXClass", "Vehicle Position - (Unknown)", nullptr));

    action_Purchase_Monthly_Sub->setText(QCoreApplication::translate("QtAtcXClass", "App Support/Enhancements- (Unknown)", nullptr));
    action_Purchase_Lifetime->setText(QCoreApplication::translate("QtAtcXClass", "Lifetime Purchase- (Unknown)", nullptr));

    action_Units_VSI_Minute->setText(QCoreApplication::translate("QtAtcXClass", "per Minute", nullptr));
    action_Units_VSI_Second->setText(QCoreApplication::translate("QtAtcXClass", "per Second", nullptr));

    action_3D_Show_You->setText(QCoreApplication::translate("QtAtcXClass", "You", nullptr));
    menu_File->setTitle(QCoreApplication::translate("QtAtcXClass", "Main", nullptr));
    menu_Mode->setTitle(QCoreApplication::translate("QtAtcXClass", "Guidance", nullptr));
    menu_View->setTitle(QCoreApplication::translate("QtAtcXClass", "View", nullptr));
    menuBlip->setTitle(QCoreApplication::translate("QtAtcXClass", "Blip ID", nullptr));
    menu_About->setTitle(QCoreApplication::translate("QtAtcXClass", "About", nullptr));
    menu_Boundaries->setTitle(QCoreApplication::translate("QtAtcXClass", "Boundaries", nullptr));
    menu_Labels_Filter->setTitle(QCoreApplication::translate("QtAtcXClass", "Label Filters", nullptr));
    menu_Settings_General->setTitle(QCoreApplication::translate("QtAtcXClass", "Line Destination", nullptr));
    menu_2D->setTitle(QCoreApplication::translate("QtAtcXClass", "2D", nullptr));
    menu_3D->setTitle(QCoreApplication::translate("QtAtcXClass", "3D", nullptr));
    menuBlip_Verbosity_3D->setTitle(QCoreApplication::translate("QtAtcXClass", "Blip Verbosity", nullptr));
    menuUnits->setTitle(QCoreApplication::translate("QtAtcXClass", "Units", nullptr));
    menuSpeed->setTitle(QCoreApplication::translate("QtAtcXClass", "Speed", nullptr));
    menuAltitude->setTitle(QCoreApplication::translate("QtAtcXClass", "Altitude", nullptr));
    menuDistance->setTitle(QCoreApplication::translate("QtAtcXClass", "Distance", nullptr));
    menuVertical_Speed->setTitle(QCoreApplication::translate("QtAtcXClass", "Vertical Speed", nullptr));
    menu_InAppPurchases->setTitle(QCoreApplication::translate("QtAtcXClass", "Purchases", nullptr));
    menu_Jump_To->setTitle(QCoreApplication::translate("QtAtcXClass", "Jump To", nullptr));
}

QString Ui_QtAtcXClass::getDistUnits() const
{
    return frameBuffer->getTFLView()->getUnits().getDistName();
}

QString Ui_QtAtcXClass::getSpdUnits() const
{
    return frameBuffer->getTFLView()->getUnits().getSpeedName();
}

QString Ui_QtAtcXClass::getAltUnits() const
{
    return frameBuffer->getTFLView()->getUnits().getAltitudeName();
}

QString Ui_QtAtcXClass::getVsiUnits() const
{
    return frameBuffer->getTFLView()->getUnits().getVsiIntervalName();
}

bool Ui_QtAtcXClass::get3DView() const
{
    return frameBuffer->getTFLView()->isView3D();
}

int Ui_QtAtcXClass::getHdgCutOff() const
{
    return frameBuffer->getTFLView()->getGPSHdgCutOffSpd();
}

void Ui_QtAtcXClass::setHdgCutOff(int value)
{
    frameBuffer->getTFLView()->setGPSHdgCutOffSpd(value);
}

int Ui_QtAtcXClass::getGPSUpdateInterval() const
{
    if( _locationInfo == nullptr)
        return 0;

    return _locationInfo->updateInterval();
}

void Ui_QtAtcXClass::setGPSUpdateInterval(int v)
{
    if( _locationInfo == nullptr)
        return;

    _locationInfo->setUpdateInterval(v);
}

float Ui_QtAtcXClass::getProximityDistance() const
{
    const Units& units = frameBuffer->getTFLView()->getUnits();

    return units.getDistance(frameBuffer->getTFLView()->getProximityDistance());
}

void Ui_QtAtcXClass::setProximityDistance(float dist)
{
    const Units& units = frameBuffer->getTFLView()->getUnits();

    return frameBuffer->getTFLView()->setProximityDistance(units.getInvDistance(dist));
}
