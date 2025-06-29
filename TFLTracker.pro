#-------------------------------------------------
#
# Project created by QtCreator 2018-09-12T16:05:24
#
#-------------------------------------------------

QT       += core gui network positioning
QT += multimedia

QT += sensors
QT += concurrent
QT += quick quickcontrols2

QT += xml

QT += core-private

TARGET = TFLTracker
TEMPLATE = app

DEFINES += QtSOAP_EXPORTS

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++17

include(sdk/jibbs.pri)
include(assimp2/assimp.pri)
include(qtsoap/qtsoap.pri)
include(qtstomp/qtstomp.pri)

INCLUDEPATH += sdk/include

SOURCES += \
    MapRenderWorker.cpp \
    MapRenderer.cpp \
    NationalRailCRC.cpp \
    NationalRailPositionProvider.cpp \
    NetworkRailScheduleJSON.cpp \
    NetworkRailStnsCSV.cpp \
    OSMData.cpp \
    OSMRenderer.cpp \
    StopPointMins.cpp \
    TFLLine.cpp \
    TFLViewFrameBuffer.cpp \
    TileManager.cpp \
        main.cpp \
    Units.cpp \
    WorldModel.cpp \
    TFLModel.cpp \
    TurnDirection.cpp \
    LineBuilder.cpp \
    Line.cpp \
    Vehicle.cpp \
    RadarSymbols.cpp \
    View3D.cpp \
    MouseAreaManager.cpp \
    MouseArea.cpp \
    FlatButtonManager.cpp \
    InAppStore.cpp \
    TrackerGlue.cpp \
    TFLView.cpp \
    TFLRenderer.cpp \
    TFLLineRenderer.cpp \
    TFLVehicleRenderer.cpp \
    LineType.cpp \
    Branch.cpp

HEADERS += \
    BranchConnect.h \
    MapRenderWorker.h \
    MapRenderer.h \
    NationalRailCRC.h \
    NationalRailPositionProvider.h \
    NetworkRailScheduleJSON.h \
    NetworkRailStnsCSV.h \
    OSMData.h \
    OSMRenderer.h \
    StopPointMins.h \
    TFLLine.h \
    TFLViewCallBack.h \
    TFLViewFrameBuffer.h \
    TileManager.h \
    TrackPoint.h \
    Units.h \
    ViewState.h \
    csvfileload.h \
    helpers.h \
    WorldModel.h \
    TFLModel.h \
    TurnDirection.h \
    LineBuilder.h \
    Line.h \
    Vehicle.h \
    RadarSymbols.h \
    View3D.h \
    BlipVerbosity.h \
    MouseAreaManager.h \
    RealTimeGPS.h \
    MouseArea.h \
    FlatButtonManager.h \
    InAppStore.h \
    TrackerGlue.h \
    TFLView.h \
    TFLRenderer.h \
    TFLLineRenderer.h \
    TFLVehicleRenderer.h \
    LineType.h \
    Branch.h \
    screenUtils.h \
    touchEventdata.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += $$files("android/*", true)


ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android

android: include(android_openssl/openssl.pri)

win32 {
scripts.files += scripts/__obbs/*.obb
scripts.path = $$DEPLOY_DIR/scripts
INSTALLS += scripts
}

shaders.files += shaders/*
shaders.path = $$DEPLOY_DIR/shaders
INSTALLS += shaders

images.files += images/earthmap.jpg
images.files += images/skydome.jpg
images.files += images/earthmap_night.jpg
images.path = $$DEPLOY_DIR/images
INSTALLS += images

models.files += models/*
models.path = $$DEPLOY_DIR/models
INSTALLS += models

objmtl.files += local_objmtl/*
objmtl.path = $$DEPLOY_DIR/objmtl
INSTALLS += objmtl

skyboxes.files += skyboxes/*
skyboxes.path = $$DEPLOY_DIR/skyboxes
INSTALLS += skyboxes

sounds.files += sounds/*
sounds.path = $$DEPLOY_DIR/sounds
INSTALLS += sounds

win32 {
openssl.files += openssl/win64/*
openssl.path = $$DEPLOY_DIR
INSTALLS += openssl
}

RESOURCES += \
    resources.qrc \
    qml.qrc \
    qtatcx.qrc

HEADERS += \
    WeakThis.h \
    UI.h

SOURCES += \
    UI.cpp


DISTFILES += $$files("shaders/*", true) $$files("android/*", true)


