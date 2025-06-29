/********************************************************************************
** Form generated from reading UI file 'qtatcx.ui'
**
** Created by: Qt User Interface Compiler version 5.11.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QTATCX_H
#define UI_QTATCX_H

#include <jibbs/action/Action.h>
#include <jibbs/action/ActionValue.h>
#include <jibbs/action/ActionGroup.h>
#include <jibbs/action/Menu.h>

#include <QtCore/QVariant>
#include <QGeoPositionInfoSource>

#include "TFLView.h"

class TFLViewFrameBuffer;

#define UI_VALUE(name) \
    Q_PROPERTY(ActionValue* name MEMBER name) \
    ActionValue* name = new ActionValue(this);

#define SHOW_MENU(name) Q_PROPERTY(Menu* name MEMBER name CONSTANT)
#define SHOW_ACTION(name) Q_PROPERTY(Action* name MEMBER name CONSTANT)
#define SHOW_ACTIONGROUP(name) Q_PROPERTY(ActionGroup* name MEMBER name CONSTANT)

class Ui_QtAtcXClass : public QObject
{
    Q_OBJECT
    SHOW_MENU(menu_Circle_ID_Color_Override)
    SHOW_MENU(menu_Labels_Filter)
    SHOW_MENU(menu_TFL_Filter)
    SHOW_MENU(menu_Settings_General)
    SHOW_ACTIONGROUP(menuUnitsSpeed)
    SHOW_ACTIONGROUP(menuUnitsDistance)
    SHOW_ACTIONGROUP(menuUnitsAltitude)
    SHOW_ACTIONGROUP(menuUnitsVsiInterval)

    SHOW_ACTIONGROUP(menu_Label_Bus_Verbosity)
    SHOW_ACTIONGROUP(menu_Label_Train_Verbosity)

    SHOW_MENU(menu_2D)

    SHOW_MENU(menu_3D)
    SHOW_MENU(menuBlip_Verbosity_3D)
    SHOW_ACTIONGROUP(actionGroup_skyLineGroup)

    SHOW_MENU(lineGroup_tube)
    SHOW_MENU(lineGroup_river_bus)
    SHOW_MENU(lineGroup_river_tour)
    SHOW_MENU(lineGroup_national_rail)
    SHOW_MENU(lineGroup_tfl_rail)
    SHOW_MENU(lineGroup_cable_car)
    Q_PROPERTY(QStringList busList MEMBER _busList NOTIFY onBusList)
    Q_PROPERTY(QStringList tflModeList MEMBER _tflModeList CONSTANT)

    SHOW_MENU(menu_View)
    SHOW_MENU(menuBlip)

    SHOW_MENU(menu_Jump_To)

    SHOW_MENU(menu_Boundaries)

    SHOW_MENU(menu_InAppPurchases)

    SHOW_ACTION(action_Show_Proximity_Rings)
    SHOW_ACTION(action_Proximity_Active)
    SHOW_ACTION(action_Proximity_Mute_sound)

    SHOW_ACTION(action_3D_VR)

    SHOW_ACTION(action_RealTime_GPS)

    SHOW_ACTION(action_Purchase_Monthly_Sub)
    SHOW_ACTION(action_Purchase_Lifetime)

    SHOW_ACTION(action_InAppCheckMe)

    Q_PROPERTY(QString unitDist READ getDistUnits)
    Q_PROPERTY(QString unitSpd READ getSpdUnits)
    Q_PROPERTY(QString unitAlt READ getAltUnits)
    Q_PROPERTY(QString unitVsi READ getVsiUnits)

    Q_PROPERTY(bool is3DView READ get3DView )

    Q_PROPERTY(int GPSHdgCutOffSpeed READ getHdgCutOff WRITE setHdgCutOff)
    Q_PROPERTY(int GPSUpdateInterval READ getGPSUpdateInterval WRITE setGPSUpdateInterval)

    Q_PROPERTY(float ProximityDist READ getProximityDistance WRITE setProximityDistance )

    Q_PROPERTY(QVariantList lineGroupBusList MEMBER lineGroupBusList NOTIFY onNotify )

public:
    explicit Ui_QtAtcXClass(QObject* parent = nullptr);

    void setupUi();
    void setGPSSource(QGeoPositionInfoSource*);

    Action *action_InAppCheckMe;

    Action *action_Exit;
    Action *action_ZoomIn;
    Action *action_ZoomOut;
    Action *actionShow;

    Action *action_Close_Simulation;
    Action *action_Debug;
    Action *action_Clock;

    Action *action_Pause;
    Action *action_About_Data_Source;
    Action *action_About_Legal;

    Action *action_Compass;
    Action *action_GPS;
    Action *action_Call_Sign;
    Action *action_Places;

    Action *action_RealTime_GPS;
    Action *action_Release_Notes;

    Action *action_Bus_Verbosity_None;
    Action *action_Bus_Verbosity_LineId;
    Action *action_Bus_Verbosity_VehicleId;
    Action *action_Bus_Verbosity_All;

    Action *action_Train_Verbosity_None;
    Action *action_Train_Verbosity_LineId;
    Action *action_Train_Verbosity_VehicleId;
    Action *action_Train_Verbosity_All;

    Action *action_Proximity_Active;
    Action *action_Proximity_Mute_sound;

    Action *action_London_Information;
    Action *action_Show_Proximity_Rings;

    Action *action_2D_Map_Night;

    Action *action_OSM_Motorway_Visible;
    Action *action_OSM_Primary_Visible;
    Action *action_OSM_Secondary_Visible;
    Action *action_OSM_Tertiary_Visible;
    Action *action_OSM_Residential_Visible;
    Action *action_OSM_Footway_Visible;
    Action *action_OSM_Water_Visible;
    Action *action_OSM_Aeroway_Visible;
    Action *action_OSM_CycleWay_Visible;
    Action *action_OSM_Pedestrian_Visible;

    Action *action_TFL_BusStop_Visible;
    Action *action_TFL_BusLine_Visible;
    Action *action_TFL_TubeLine_Visible;

    Action *action_piccadillyNormalDestination;

    Action *action_Use_Vehicle_Behaviour;

    Action *action_Elizabeth_Arrivals_Use_TFL_Data;

    Action *action_Instruction_Notes;
    Action *action_Display_Short_Destination;

    Action *actionPurchase;
    Action *actionIsPurchased;
    Action *action_android_test_purchased;
    Action *action_android_test_canceled;
    Action *action_android_test_refunded;
    Action *action_android_test_item_unavailable;
    Action *action_3D_VR;

    Action *action_3D_Blip_Verbosity_None;
    Action *action_3D_Blip_Verbosity_ID;
    Action *action_3D_Blip_Verbosity_All;

    Action *action_Units_Spd_Knots;
    Action *action_Units_Spd_Km_H;
    Action *action_Units_Spd_MPH;
    Action *action_Units_Alt_Feet;
    Action *action_Units_Alt_Meters;
    Action *action_Units_Dist_SM;
    Action *action_Units_Dist_KM;
    Action *action_Units_Dist_Meters;
    Action *action_Units_Spd_Meters_Second;

    Action *action_Units_Alt_Nautical_Miles;
    Action *action_Units_Alt_Statue_Miles;
    Action *action_Units_Alt_Kilometers;

    Action *action_InApp_3DView;
    Action *action_InApp_RealTimeGPS;
    Action *action_InApp_ProximityAlert;
    Action *action_InApp_VehiclePosition;

    Action *action_Purchase_Monthly_Sub;
    Action *action_Purchase_Lifetime;

    Action *action_Units_VSI_Minute;
    Action *action_Units_VSI_Second;

    Action *action_3D_Show_You;

    Menu *menuBar;
    Menu *menu_File;
    Menu *menu_Mode;
    Menu *menu_View;
    Menu *menuBlip;
    Menu *menu_About;
    Menu *menu_Boundaries;

    Menu *menu_Labels_Filter;
    Menu *menu_TFL_Filter;

    Menu *menu_Circle_ID_Color_Override;

    Menu *menu_Settings_General;
    Menu *menu_2D;
    Menu *menu_3D;
    Menu *menuBlip_Verbosity_3D;
    Menu *menuUnits;
    Menu *menuSpeed;
    Menu *menuAltitude;
    Menu *menuDistance;
    Menu *menuVertical_Speed;
    Menu *menu_InAppPurchases;
    Menu *menu_Jump_To;

    ActionGroup* menu_Label_Bus_Verbosity;
    ActionGroup* menu_Label_Train_Verbosity;
    ActionGroup* menu3DBlipVerbosity;
    ActionGroup* menuUnitsSpeed;
    ActionGroup* menuUnitsDistance;
    ActionGroup* menuUnitsAltitude;
    ActionGroup* menuUnitsVsiInterval;
    ActionGroup* menu2DRadarProfile;
    ActionGroup* actionGroup_skyLineGroup;

    Menu* lineGroup_coach;
    Menu* lineGroup_cycle;
    Menu* lineGroup_cycle_hire;
    Menu* lineGroup_replacement_bus;
    Menu* lineGroup_river_bus;
    Menu* lineGroup_river_tour;
    Menu* lineGroup_taxi;
    Menu* lineGroup_tube;
    Menu* lineGroup_national_rail;
    Menu* lineGroup_tfl_rail;
    Menu* lineGroup_cable_car;

    QVariantList lineGroupBusList;

    QStringList _busList;
    QStringList _tflModeList;

    QGeoPositionInfoSource* _locationInfo = nullptr;
    TFLViewFrameBuffer* frameBuffer = nullptr;

    Q_INVOKABLE void trigger(const QString& objectName);
    Q_INVOKABLE void toggle(const QString& objectName, bool value);

signals:
    void onNotify(QVariantList);
    void onBusList(QStringList);

private:
    void retranslateUi();

    QString getDistUnits() const;
    QString getSpdUnits() const;
    QString getAltUnits() const;
    QString getVsiUnits() const;

    float getProximityDistance() const;
    void setProximityDistance(float dist);

    bool get3DView() const;

    int getHdgCutOff() const;
    void setHdgCutOff(int);

    int getGPSUpdateInterval() const;
    void setGPSUpdateInterval(int v);

};


#endif // UI_QTATCX_H
