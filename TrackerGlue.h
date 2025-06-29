#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <jibbs/utilities/permissions.h>
#include <jibbs/vector/vector3.h>
#include <jibbs/maptiles/MapTileEntries.h>
#include <jibbs/utilities/ISettingsPersist.h>

#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>
#include <QNetworkAccessManager>
#include <QPair>
#include <QCompass>
#include <QFuture>
#include <QElapsedTimer>
#include <QObject>
#include <QKeyEvent>
#include <QGeoPositionInfoSource>
#include <QSoundEffect>
#include <qtsoap.h>

#include "OSMData.h"
#include "WorldModel.h"

class Action;
class ActionGroup;
class ActionValue;
class Menu;
class TFLLine;
class Line;
class StopPointMins;
class InAppStore;
class QtTextureManager;
class Ui_QtAtcXClass;
class TFLViewFrameBuffer;
class NationalRailPositionProvider;

#define COMPASS_MEASUREMENTS 30

class TrackerGlue : public QObject, public ISettingsPersist
{
    Q_OBJECT

    enum class DownloadTurn {BEGIN, TFLOrNetworkRail, END};

    struct cloudEntry {
        QString letter;
        QString lineId;
    };

public:
    explicit TrackerGlue(QObject *parent = nullptr);
    ~TrackerGlue() override;

    Ui_QtAtcXClass *getUI();


    enum Duration {
        TOAST_SHORT = 0,
        TOAST_LONG = 1
    };

    Q_INVOKABLE QString textFromFile(QString filename);
    Q_INVOKABLE QString creditInfo();
    Q_INVOKABLE QString getOpenGLVersion() const;
    Q_INVOKABLE QString currentYear();
    Q_INVOKABLE void inAppPurchase(QString productID);

    Q_INVOKABLE QString uniqueAndroidID();
    Q_INVOKABLE void saveSettingsInFuture();

    Q_INVOKABLE void updateFetchFilterList();
    Q_INVOKABLE void updateNationalRailList();

    Q_INVOKABLE void showToast(const QString &message, Duration duration = TOAST_LONG);
    Q_INVOKABLE void setUserUIActive(bool isActive);
    Q_INVOKABLE void setViewFrameBuffer(TFLViewFrameBuffer *frameBuffer);

    Q_INVOKABLE TFLLine *getTFLLine(QString id, bool rebuild=false);
    Q_INVOKABLE bool doesBusRouteExist(QString id) const;
    Q_INVOKABLE void buildBusList();
    Q_INVOKABLE void rebuildBuses(QVariant ids);
    Q_INVOKABLE int getFetchListCount() const;
    Q_INVOKABLE int getFetchListLimit() const;
    Q_INVOKABLE QStringList getLineNamesForStopId(QString id) const;
    Q_INVOKABLE QStringList getLineIDsforStopId(QString id) const;

    Q_INVOKABLE void updateArrivalsForStopPoint(QString id, QStringList lineWanted);
    Q_INVOKABLE void updateLineModeStatusResults(QString mode);
    Q_INVOKABLE void updateTubeDisruptionResults();
    Q_INVOKABLE void updateDisruptionForStopPoint(QString id);
    Q_INVOKABLE void updateStatusForLines(QString mode, QStringList lines);
    Q_INVOKABLE void updateNationalRailArrivalsForStopPoint(QString id);
    Q_INVOKABLE void updateVehicleArrivalInfo(QString lineId, QString vehicleId, QString stationName);

    Q_INVOKABLE void downloadBusRoute(QString lineId);

    Q_INVOKABLE QString getMonthlySubscriptionCost();
    Q_INVOKABLE QString getLifeTimeSubscriptionCost();
    Q_INVOKABLE bool isMonthlySubscriptionPurchased();
    Q_INVOKABLE bool isLifeTimeSubscriptionPurchased();
    Q_INVOKABLE bool isMe() const;

    Q_INVOKABLE MapTileEntries* getMapTileProfiles();


protected:
    // IQSettingsPersist
    void saveSettings() Q_DECL_OVERRIDE;
    void loadSettings() Q_DECL_OVERRIDE;
    bool eventFilter(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;

private slots:
    void close();
    void keyPressEvent(QKeyEvent*);
    void onInitialise();
    void onDisplayInitialised();
    void onUnintialise();
    void updateCache();
    void onZoomIn();
    void onZoomOut();
    void onWantGPSUpdate();
    void onGPSUpdated(const QGeoPositionInfo &geoPositionInfo);
    void onNationalRailResponse(const QtSoapMessage &);
    void showPurchasePage();

signals:
    void completed();
    void showReleaseNotes();
    void qmlPromptProductPurchase(QString,QString);
    void qmlPopUp(QString title, QString msg);
    void arrivalsUpdated(QVariantMap);
    void vehicleArrivalInfoUpdated(QJsonObject);
    void tubeStatusUpdated(QVariantList);
    void tubeDisruptionUpdated(QVariantList);
    void stopPointDisruptionUpdated(QVariantMap);
    void lineStatusUpdate(QVariantList);
    void nationalRailUpdate(QJsonObject);
    void tflRouteDownloaded(QString route);
    void tflRouteFailedToDownload(QString route);
    void showQMLPage(QString, QVariantList);
    void productPurchased(QString, bool);
    void allLinesAdded(QVector<QString> ids);

private:
    void loadOBBs();
    void loadOSMData();

    void setFeedingOnline(bool bAllowed);
    void setupFetchTimer();
    void loadTFLModes();
    void tflFetch();
    void stompFetch();
    void initBlipVerbosityMenus();
    void initInAppStore();
    void connectOSM();
    void triggerOSM();
    void initReleaseNotes();
    void initUnitsMenus();
    void initFilterLabelsMenus();
    void setupGPS();
    void write_log_file(QString lineId, const QByteArray &json);
    QVariantMap parseJSONArrivals(QJsonArray array);
    QJsonObject parseJSONVehicle(QString lineId, QString stationName, QJsonArray array);
    std::shared_ptr<TFLLine> buildTFLLine(QString id);
    void prepareAllRoutes();
    QNetworkReply *downloadBusRoute(QString lineId, QString direction);
    void saveRoute(const QByteArray &json);
    void setUpFetchList();
    void updateStompSubscribeList();
    void setupHardcodedColors();

    std::shared_ptr<Line> getLine(const QString& direction, const QString &id);
    void addRoute(std::shared_ptr<Line> line, std::shared_ptr<StopPointMins> timeData);

    bool doesRouteExist(QString id) const;
    bool allRoutesListsEmpty() const;
    QStringList getScannedModes() const;
    void storeAllRouteIDsInList(const QByteArray &json);
    void loadAllRoutes();
    bool loadRoute(QString line);
    void loadRouteFromPath(const QString routePath, const QString timePath);

    bool getPiccHeathrowDestnNormalFormat() const;
    void checkNationalRailStns();

    void setGlueReady(bool fullyReady=false);

private:
    const QString _allAvailableBusRoutesURL = "https://api.tfl.gov.uk/Line/Mode/bus,tube,tram,national-rail,dlr,overground,elizabeth-line,river-bus,coach,river-tour,cable-car";
    const QString _lineArrivalsURL = "https://api.tfl.gov.uk/line/%1/arrivals";
    const QString _stopPointArrivalsURL = "https://api.tfl.gov.uk/stoppoint/%1/arrivals";
    const QString _stopPointLineArrivalsURL = "https://api.tfl.gov.uk/line/%1/arrivals/%2";
    const QString _lineModeStatusURL = "https://api.tfl.gov.uk/line/mode/%1/status";
    const QString _tubeDisruptionURL = "https://api.tfl.gov.uk/StopPoint/mode/%1/Disruption";
    const QString _stopPointDisruptionURL = "https://api.tfl.gov.uk/StopPoint/%1/Disruption";
    const QString _lineStatusURL = "https://api.tfl.gov.uk/line/%1/status";
    const QString _routeSequence = "https://api.tfl.gov.uk/Line/%1/Route/sequence/%2";
    const QString _vehicleArrivalURL = "https://api.tfl.gov.uk/Vehicle/%1/Arrivals";
    
    Ui_QtAtcXClass* ui;
    std::vector<QByteArray> _obbByteArrays;
    QHash<QString,std::shared_ptr<TFLLine>> _rootLines;
    QNetworkAccessManager* _manager = nullptr;
    QGeoPositionInfoSource* _locationInfo = nullptr;
    QByteArray _userInfo;
    QCompass* _compass;
    Vector3F _compassMeasurements[COMPASS_MEASUREMENTS] ={};
    size_t _compassIdx = 0;

    bool _bAppFeedingLive = true;
    bool _bInitialised = false;
    bool _bDisplayInitialised = false;
    bool _bSettingsLoaded = false;
    bool _bUserUIMode = false;
    bool _bWantToSave = false;
    int _showReleaseNotesMins = -1;
    bool _elizabethLineUsesTFL = false;

    QSoundEffect* _trafficAlert;
    QTimer* _timerTrafficAlert;

    QTimer* _perSecondTimer = nullptr;
    QTimer* _fetchTimer = nullptr;
    QVector<QString> _NationalRailList;

    std::shared_ptr<OSMData> _osmData = std::make_shared<OSMData>();
    std::shared_ptr<WorldModel> _model = std::make_shared<WorldModel>();

    QSet<QString> _fetchList;
    QSet<QString> _fetchNationalRailList;
    std::map<QString, QString> _lineColorMap;
    std::map<QString, int> _lineOffsetMap;

    bool            _currentlyDownloading = false;
    DownloadTurn _downloadTurn = DownloadTurn::BEGIN;

    InAppStore* _inAppStore = nullptr;
    std::map<QString,Action*> _inAppProductMap;

    Permissions _permissions;

    const int _maxInAppVehicleCount = 2;

    QtSoapHttpTransport _soapHttp;
    std::unique_ptr<NationalRailPositionProvider> _stompNationalRail;
    MapTileEntries* _mapTilesEntries = new MapTileEntries(this, "Settings/TileMapServers/");

    bool _allLoadedRoutesRefreshed = false;
    QVector<QString> _allLoadedRoutesList;
    QSet<QString>   _allRoutesList;
    QSet<QString>   _allRoutesNamesList;
    QSet<QString>   _allBusRoutesList;

    const QString routesFolder = ":/data/Routes";
    const QString downloadRoutesFolder = "downloads/routes";

    QFuture<void> _loadFuture;
    QFutureWatcher<void>* _watcher = new QFutureWatcher<void>(this);
};

#endif // MAINWINDOW_H
