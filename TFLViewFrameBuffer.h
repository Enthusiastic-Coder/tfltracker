#pragma once

#include <jibbs/utilities/IPersist.h>
#include <jibbs/gps/GPSLocation.h>

#include <QWaitCondition>
#include <QQuickFramebufferObject>
#include <QMutex>
#include <QTimer>

#include "TFLViewCallBack.h"

class TFLView;

class TFLViewFrameBuffer : public QQuickFramebufferObject, public IQSettingPersist
{
    Q_OBJECT
    QML_NAMED_ELEMENT(TFLView)
public:
    explicit TFLViewFrameBuffer();
    ~TFLViewFrameBuffer();

    void isViewReady(std::function<void (bool)> callback);
    Renderer *createRenderer() const Q_DECL_OVERRIDE;

    void executeOnRenderThread(TFLViewCallback callback, bool doUpdate=true);

    Q_INVOKABLE void updateOSM();
    Q_INVOKABLE void triggerArrivalStatusPage(QString id, QString name, QString stopLetter, QString towards);
    Q_INVOKABLE void triggerArrivalNationalRailStatusPage(QString CRC, QString name);
    Q_INVOKABLE void selectVehicle(QString id);
    Q_INVOKABLE void tellFullScreen(bool);

    TFLView* getTFLView();

    Q_INVOKABLE void hideRadarView(int ms);
    Q_INVOKABLE void persistenceOff();

    // IQSettingsPersist
    void saveSettings() override;
    void loadSettings() override;

protected://overrrides
    void mousePressEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void mouseMoveEvent(QMouseEvent *) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *) Q_DECL_OVERRIDE;
    void touchEvent(QTouchEvent *event) Q_DECL_OVERRIDE;

signals:
    void showMenuOptions();
    void wantShowNormal();
    void wantShowMaximum();
    void rendererCreated();
    void postCallBack(TFLViewCallback type);
    void showQMLSelectStopPointPage(QVariantList cppArgs);
    void showQMLPage(QString page, QVariantList cppArgs);

    void wantToGo(GPSLocation);
    void onWantGPSUpdate();
    void onWantRealTimeGPS(bool);
    void onWantCompassMode();
    void wantVR();
    void wantShow3D(bool);
    void onDisplayInitialised();
    void wantToggleColor();
    void wantMuteProximity(bool mute);
    void wantToSubscribe();

    void activate3DVR();

    void onProximityWarningActive(bool);

private:
    QMutex _mutexReady;
    bool _viewIsReady = false;
    TFLView* _tflView = nullptr;
    Qt::HANDLE _frameThreadID;
    QTimer* _keepAliveTimer = nullptr;
};

