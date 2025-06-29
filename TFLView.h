#ifndef TFLVIEW_H
#define TFLVIEW_H

#include <jibbs/utilities/IPersist.h>

#include <QQuickPaintedItem>
#include <QOpenGLFunctions>
#include <QImage>
#include <QOpenGLPaintDevice>
#include <QQuickFramebufferObject>
#include <QElapsedTimer>

#include "TileManager.h"
#include "View3D.h"
#include "MouseAreaManager.h"
#include "RealTimeGPS.h"
#include "FlatButtonManager.h"

#include "TFLLineRenderer.h"
#include "TFLVehicleRenderer.h"
#include "MapRenderer.h"
#include "WorldModel.h"

#include "TFLViewCallBack.h"
#include "ViewState.h"
#include "qtmetamacros.h"
#include "touchEventdata.h"

class QtTextureManager;
class WorldModel;
class MapRenderWorker;



class TFLView : public QObject, public QQuickFramebufferObject::Renderer, protected QOpenGLFunctions, public IQSettingPersist
{
    Q_OBJECT
public:

    friend class View3D;
    friend class TFLLineRenderer;///////TO REMOVE
    friend class TFLVehicleRenderer;
    friend class MapRenderer;
    friend class OSMRenderer;

    enum CallSignID{ callSign, Reg, ICAO };

    enum class ZoomLevel { VeryLow, Low, Medium, High, VeryHigh };

    enum class RealTimeMode { None, GPS, Compass };

    QOpenGLFramebufferObject *createFramebufferObject(const QSize &size) override;

    explicit TFLView(float dpr);
    virtual ~TFLView();

    void setDataDirty();

    int width() const;
    int height() const;

    Q_INVOKABLE void updateOSM();
    Q_INVOKABLE void triggerArrivalStatusPage(QString id, QString name, QString stopLetter, QString towards);
    Q_INVOKABLE void triggerArrivalNationalRailStatusPage(QString CRC, QString name);
    Q_INVOKABLE void selectVehicle(QString id);
    Q_INVOKABLE void setBatteryInfo(QString percent, bool charging);

    void setGlueReady();
    void setReady(bool bReady);
    void setUserInfo(QByteArray info);
    void setModel(std::shared_ptr<WorldModel> model);
    void setOSMData(std::shared_ptr<OSMData> data);

    float getPixelLevel() const;

    EulerF getCamera() const;
    float get3DZoom() const;
    std::pair<GPSLocation,GPSLocation> getBoundaryView(bool applyTransform=true) const;
    std::pair<GPSLocation,GPSLocation> getRadiusBoundaryView(float meters) const;
    void applyCameraPitchBounds();
    void updateViewBox(bool bForced=false);

    bool isPaused() const;
    bool isReady() const;
    bool isView3D() const;

    void setView3D(bool b3D);

    std::shared_ptr<View3D> get3D();

    const QSize& size() const;
    QRect geometry() const;
    QRect rect() const;

    void parseLineArrival(QByteArray data);
    void parseLineArrival(QByteArray data, QString lineId);
    void parseLineArrival(const NationalRailPositionProvider::Train &t);
    void removeAllVehicles();
    void removeVehicles(QString line);
    void setUseVehicleBehaviour(bool checked);
    void setPiccHeathrowDestnNormalFormat(bool checked);
    void setCircleColorOverride(QString id, bool on);
    void removeOldVehicles();

    void updateTime();

    void set3DZoom(float f);
    void setCamera(const EulerF &e);
    void setCompassValue(float value, bool bInstant=false);
    void updateTransformCompass();
    void applyCompassTransform(QPainter& p, bool bScale=false);

    void showSubscribeButton(bool show);

    void setShowLondonMetro(bool show);
    bool getShowLondonMetro() const;

    void setBusStopVisible(bool show);
    bool isBusStopVisible() const;

    void setBusLinesVisible(bool show);
    bool isBusLinesVisible() const;

    void setTubeLinesVisible(bool show);
    bool isTubeLinesVisible() const;

    void setBusBlipVerbosity(BlipVerbosity v);
    void setTrainBlipVerbosity(BlipVerbosity v);

    void setGPSHdgCutOffSpd(int spd);
    int getGPSHdgCutOffSpd() const;

    void setGPSLimitBoundary(std::pair<GPSLocation,GPSLocation> b, bool active);
    void setMyLocation(const GPSLocation &location);
    void setGPSOrigin(const GPSLocation &origin, float pm = -1, bool bInstant=false, bool bUpdateCache=false);
    void setRealTimeGPSInfo(const RealTimeGPSNewInfo &info);
    void TranslatePosition(QPoint diff);

    GPSLocation myLocation() const;
    GPSLocation gpsOrigin() const;
    float getCompassValue() const;

    void setRealTimeMode(RealTimeMode b);
    bool isRealTimeGPS() const;
    bool isRealTimeCompass() const;

    void setProximityDistance(float nm);
    void setShowProximityRings(bool b);
    void setProximityActive(bool bActive, bool bSound);

    float getProximityDistance() const;
    bool getProximityActive() const;
    bool getShowProximityRings() const;
    bool isProximityWarningInProgress() const;

    void setGPSUpdateInterval(int interval);

    void setBlockRendering(bool block);
    bool isBlockRendering() const;

    void setShowTopLeftInfo(bool bShow);

    void setInAppEvalMsgs(const std::vector<QString>& evals);
    void increment3dHeight(float fIncr);
    void onZoom(float fDiff);

    QSize middleOfScreen() const;

    // IQSettingsPersist
    void saveSettings() override;
    void loadSettings() override;

    MapRenderer& getOSMRenderer();

    Units& getUnits();
    const QString& getDataSourceName() const;
    bool hasGPSTimedOut() const;

    QString getBusyMsg();

    void setHttpLastErrMsg(QString errMsg);

    QPoint toScreen(const GPSLocation & location, bool bApplyTransform=false) const;

    QRect boundaryFromRange(const ViewState& viewState, float meters) const;
    QRect boundaryFromRange(float meters) const;

    void setMapNight(bool night);
    bool isMapNight() const;

    void setTilesDirty();

    void updateCache();

    void setRenderingIsMultithreaded(bool b);
    void callBack(TFLViewCallback viewBack);

    void tellFullScreen(bool);
    void mousePressHandler(QPoint pos);
    void mouseMoveHandler(QPoint pos);
    void mouseReleaseHandler(QPoint pos);
    void wheelHandler(QPoint pt);
    void touchHandler(touchEventData data);

    QString getOpenGLInfo() const;

protected:

    void paintGL2D();
    void paintTFL(QPainter& p);

    void paintLoadingInfo(QPainter& p);
    void paintScreenMessage(QPainter& p, QString msg);

    void paintPerimeterHdgs(QPainter &p);
    void paintRadarBottomMessage(QPainter & p);
    void paintRealTimeGPSInfo(QPainter &p);
    void paintUserUI(QPainter &p);
    void paintHttpInfo(QPainter &p);
    void paintServerInfo(QPainter &p, int x, int y);
    void paintPausedInfo(QPainter &p);
    void paintThumbnailInfo(QPainter& p);
    void paintProximity(QPainter &p);
    void paintInAppMsgs(QPainter &p);
    void paintRings(QPainter &p);
    void paintMiddle(QPainter &p);
    void paintRealGPSHistory(QPainter &p);
    void paintTiles(QPainter &p);
    void paintRawOSMTiles(QPainter &pLocal, QSize scaledSize, QColor colGrnd);

    bool isProximityWarning(const std::shared_ptr<const Vehicle> &vehicle) const;

    const QImage& getTubeStnImage() const;
    const QImage& getBusStnImage() const;
    const QImage& getTramStnImage() const;
    const QImage& getOvergroundStnImage() const;
    const QImage& getDLRStnImage() const;
    const QImage& getElizabethRailStnImage() const;
    const QImage& getNationalRailImage() const;
    const QImage& getRiverBusImage() const;
    const QImage& getIFSCableCarImage() const;

    qreal getCurrentScale() const;

    ViewState createToScreenSnapshot() const;

protected slots:
    void onZoomIn();
    void onZoomOut();
    void onFlatButtonPressed(QString id);

protected:
    void render() override;

signals:
    void aboutToBeDestroyed();
    void onDisplayInitialised();
    void onWantGPSUpdate();
    void onWantRealTimeGPS(bool);
    void onWantCompassMode();
    void wantVR();
    void wantShowMaximum();
    void wantShowNormal();
    void wantShow3D(bool);
    void wantToggleColor();
    void wantMuteProximity(bool mute);
    void showMenuOptions();
    void emitBusyMsg(QString msg);
    void onSwitchView3D(bool is3D);
    void showQMLPage(QString page, QVariantList cppArgs);
    void showQMLSelectStopPointPage(QVariantList cppArgs);
    void wantToSubscribe();
    void wantToGo(GPSLocation);
    void requestMapRender(ViewState snapshot, QByteArray hash);
    void onProximityWarningActive(bool);

private:
    std::shared_ptr<WorldModel> getWorldModel();

    void paintNative(float dt);
    void paint(QPainter &p);
    void sync();
    void initLinesLogo();
    void initOSM();

    void recordOpenGLInfo();
    bool ptInScreen(const GPSLocation &loc) const;
    bool ptInScreen(const QPoint& pt) const;
    QPoint invPtInScreen(const QPoint& pt);
    GPSLocation toGPS(const QPoint& pt, bool bApplyTransform=false) const;
    bool isTotallyReady() const;
    void loadResourcesIfRequired();
    bool updateCompassValue(float fDt);
    bool updateGPSValue(float fDt);
    void updateFlatButtonVisibility();
    void onFinalisePixelPerMile(float fPixelsPerMile);
    QString GetRadarAltText(float ft, int dp=2);
    void addFlatButtons();
    void setBusyMsg(QString msg);
    bool refresh3DBlipSelected(const QPoint &topLeft, const QPoint &bottomRight);
    bool refresh2DBlipSelected(GPSLocation topLeft, GPSLocation bottomRight);
    void refreshBlipSelected();
    void updateSelectedVehicleVector();
    ZoomLevel getZoomLevel(float pixm) const;
    ZoomLevel getZoomLevel() const;

    void startMapRendering();

    void hdgAreaPaintHandler(QPainter&p, bool isMouseDown, const QRect &rc);
    void altAreaPaintHandler(QPainter&p, bool isMouseDown, const QRect &rc);
    void pitchAreaPaintHandler(QPainter&p, bool isMouseDown, const QRect &rc);
    void vertMouseAreaPainter(QPainter& p, QRect rc, int arrlen, bool isMouseDown);

private slots:
    void mapImageRendered(const QImage image, const QByteArray hash);

private:
    const Vector3F brown = Vector3F( 240,240,19);
    const Vector3F green = Vector3F(4, 197, 18);
    const Vector3F white = Vector3F(255,255,255);
    const Vector3F lightBlue = Vector3F(66,244,234);
    const QString _aircraftThumnailDownloadURL = "http://www.airport-data.com/api/ac_thumb.json";
    const QString _strUp = QString::fromUtf16((const char16_t*)L"\u2191");
    const QString _strDown = QString::fromUtf16((const char16_t*)L"\u2193");

    const QLatin1String _flatSatId = QLatin1String("satellite");
    const QLatin1String _flatShowMax = QLatin1String("showmaxscreen");
    const QLatin1String _flatShowNormal = QLatin1String("shownormscreen");
    const QLatin1String _flatShow2D = QLatin1String("show2D");
    const QLatin1String _flatShow3D = QLatin1String("show3D");
    const QLatin1String _flatResetCompass = QLatin1String("resetCompass");
    const QLatin1String _flatZoomIn = QLatin1String("zoomin");
    const QLatin1String _flatZoomOut = QLatin1String("zoomout");
    const QLatin1String _flatMainMenu = QLatin1String("mainmenu");
    const QLatin1String _flatVR = QLatin1String("VR");
    const QLatin1String _flatRadarToggleColor = QLatin1String("radarToggleColor");
    const QLatin1String _flatProxmOn = QLatin1String("proximityOn");
    const QLatin1String _flatProxmOff = QLatin1String("proximityOff");
    const QLatin1String _flatSubscription = QLatin1String("subscription");
    const QLatin1String _flatWest = QLatin1String("west");
    const QLatin1String _flatEast = QLatin1String("east");

    BlipVerbosity _busBlipVerbosity = BlipVerbosity::All;
    BlipVerbosity _trainBlipVerbosity = BlipVerbosity::All;

    bool _bSettingsLoaded = false;

    QByteArray _userInfo;

    QElapsedTimer _elapsedDisplayTimer;
    QScopedPointer<QOpenGLPaintDevice> m_device;
    bool _bUpdateData = true;
    QTimer* _dirtyFlagTimer = nullptr;
    bool _bPaused = false;
    bool _bIsReady = false;
    bool _bBlockRendering = false;
    bool _bResourcesLoaded = false;

    float _devicePixelRatio = 1.0f;

    QString _openGLInfo;
    bool _isMultiThreaded = false;

    QString _batteryCharge = 0;
    bool _batteryStatus = 0;

    GPSLocation _gpsOrigin = GPSLocation(51.5117, -0.18595,1000);
    GPSLocation _gpsOriginTarget = _gpsOrigin;

    float _PixelsPerMile = 650.0f;
    float _PinchPixelsPerMile = -1;
    QPoint _mouseLastPoint;
    bool _bInPinch = false;
    bool _bMouseDown = false;
    bool _bMouseCapture = false;
    bool _b3DMode = false;
    bool _bShowLondonMetro = false;
    bool _showTopInfo=true;
    bool _bIsFullScreen=false;
    QString _lastConnectionError;

    QString _strBusyMsg = "processing";

    GPSLocation _myLocation;

    RealTimeMode _bRealTimeMode = RealTimeMode::None;

    float _pinchCompassReading = 0.0f;
    float _finalCompassReading=0.0f;
    QTransform _compassTransform;
    QTransform _invCompassTransform;

    QImage _tubeStnImg;
    QImage _busStnImg;
    QImage _tramStnImg;
    QImage _overGrndStnImg;
    QImage _DLRStnImg;
    QImage _TFLRailStnImg;
    QImage _NationalRailStnImg;
    QImage _RiverBusStnImage;
    QImage _IFSCableCarImage;

    std::shared_ptr<QtTextureManager> _texManager = std::make_shared<QtTextureManager>("TFLView");

    std::vector<QString> _inAppEvalMsgs;

    std::shared_ptr<View3D> _3d = std::make_shared<View3D>(this);

    Units _units;

    int _topHeightWanted = 0;
    int _bottomHeightWanted = 0;
    int _perimeterHeightWanted = 0;
    bool _resizeWanted = true;

    FlatButtonManager _flatButtonManager;
    bool _bLookingforGPS = false;

    bool _rotUnstuck = false;
    bool _lockRotationForEver = false;

    QVector<QPoint> _beaconPts;

    QTime _lastTime;

    float _3dZoomFactor=1.0f;
    float _pinch3DzoomFactor=1.0f;
    EulerF _camera;
    float _lastHeight = 0.0f;

    MouseAreaManager _mouseAreaManager;

    QString _displayMessageAtBottom;

    QTime _lastTimeGPSClicked;
    int _gpsMinSpdSensitivity = 0;
    RealTimeGPSInfo _realGPSInfo;
    int _updateGPSInterval;
    QTime _lastUpdateTime;

    Vector3F _blipVector;

    float _compassTurnRate = 0.0f;

    GPSLocation _gpsClickLocation;
    qreal _currentScaleFactor = 1.0;

    TFLLineRenderer _lineRenderer;
    MapRenderer _osmRenderer;

    QThread *_mapWorkerThread;
    MapRenderWorker *_mapWorker;

    QImage _OSMImg;
    QImage _VehicleImg;
    bool _rebuildOsmImg = true;

    TFLVehicleRenderer _vehicleRenderer;

    std::shared_ptr<WorldModel> _model;

    float _proximityDist = 0.25f;
    bool _proximityActive = false;
    bool _proximityActiveSound = false;
    bool _bShowProximityRings = false;
    bool _proximityWarningInProgress = false;

    bool _mapNight = false;

    bool _gpsLimitBoundaryActive = false;
    std::pair<GPSLocation,GPSLocation> _gpsLimitBoundary;

    TileManager _tileManager;

    struct tileData {
        QImage img;
        QRect target;
    };

    std::map<QString,tileData> _tileImages;

    /////////////////QML
    QSize _size;
    QSize _prevSize;
    bool _initGLDone = false;
    bool _glueReady = false;
};

#endif // TFLVIEW_H
