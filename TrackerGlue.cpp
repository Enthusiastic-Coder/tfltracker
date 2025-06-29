#include <jibbs/utilities/stdafx.h>
#include <jibbs/maptiles/MapTileEntry.h>
#include <jibbs/android/assetpack.h>

#include <QKeyEvent>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QtXml>
#include <QDirIterator>
#include <QDir>
#include <QFileInfo>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QTimer>
#include <QQuickWindow>
#include <QDebug>
#include <QSet>
#include <QGeoPositionInfo>
#include <QRandomGenerator>
#include <QDateTime>
#include <QDate>
#include <QQmlEngine>
#include <QMetaObject>

#include <algorithm>

#include "Line.h"
#include "TFLLine.h"
#include "helpers.h"
#include "InAppStore.h"
#include "UI.h"
#include "TrackerGlue.h"
#include "TFLViewFrameBuffer.h"

#include "StopPointMins.h"
#include "LineBuilder.h"
#include "WorldModel.h"


//#define WANT_NATIONAL_RAIL
//#include "NationalRailPositionProvider.h"


#ifdef Q_OS_WIN
#define NOMINMAX
#include <Windows.h>
#endif


static const QString appID = "6fb298fd";
static const QString key = "b9434ccf3448ff8def9d55707ed9c406";

namespace {

bool busNumberComparerId(const  QStringView& leftId, const  QStringView& rightId)
{
    bool leftIsLetter = leftId[0].isLetter();
    bool rightIsLetter = rightId[0].isLetter();

    if( leftIsLetter != rightIsLetter)
        return rightIsLetter;

    if( leftIsLetter && rightIsLetter)
    {
        if( leftId[0] == rightId[0])
        {
            if( leftId[1].isLetter() && rightId[1].isLetter() )
            {
                if(leftId[1] == rightId[1])
                    return leftId.mid(2).toInt() < rightId.mid(2).toInt();
            }
            return leftId.mid(1).toInt() < rightId.mid(1).toInt();
        }
        else
            return leftId[0] < rightId[0];
    }

    return leftId.toInt() < rightId.toInt();
}

bool busNumberComparer(const QJsonObject& l, const QJsonObject& r)
{
    QString leftId = l["id"].toString();
    QString rightId = r["id"].toString();
    return busNumberComparerId(leftId, rightId);
}
}

TrackerGlue::TrackerGlue(QObject *parent) :
    QObject(parent),
    ui(new Ui_QtAtcXClass(this))
{
    _manager = new QNetworkAccessManager(this);

    loadOBBs();
}

TrackerGlue::~TrackerGlue()
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif
    _loadFuture.cancel();
    _loadFuture.waitForFinished();

    delete ui;
}

Ui_QtAtcXClass *TrackerGlue::getUI()
{
    return ui;
}

QString TrackerGlue::textFromFile(QString filename)
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO << ":" << filename;
#endif
    QFile f(filename);
    if( f.open(QIODevice::ReadOnly))
        return f.readAll();

    return filename + " - not found";
}

QString TrackerGlue::creditInfo()
{
    QSettings s;
    QString strBuildDateTime = s.value(BuildVersion::BuildDateTime, "").toString();
    return QString("%1<br>%2<br>buildCpu:[%3]<br>currentCpu:[%4]<br>Qt:[%5]")
            .arg(strBuildDateTime)
            .arg(uniqueAndroidID())
            .arg(QSysInfo::buildCpuArchitecture())
            .arg(QSysInfo::currentCpuArchitecture())
            .arg(QT_VERSION_STR);
}

QString TrackerGlue::getOpenGLVersion() const
{
    if( ui->frameBuffer == nullptr)
        return QLatin1String("OpenGL not loaded yet");

    return ui->frameBuffer->getTFLView()->getOpenGLInfo();
}

QString TrackerGlue::currentYear()
{
    return QString::number(QDate::currentDate().year());
}

void TrackerGlue::inAppPurchase(QString productID)
{
    if( _inAppStore != nullptr)
        _inAppStore->purchase(productID);
}

void TrackerGlue::close()
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif

    if( ui->frameBuffer != nullptr)
        ui->frameBuffer->window()->close();
}

void TrackerGlue::setViewFrameBuffer(TFLViewFrameBuffer *frameBuffer)
{
    ui->frameBuffer = frameBuffer;

    auto startInitialise = [this] {

        ui->frameBuffer->getTFLView()->setModel(_model);
        ui->frameBuffer->getTFLView()->setOSMData(_osmData);
        ui->frameBuffer->loadSettings();

        if( _bInitialised)
        {
            setGlueReady(true);
        }
        else
        {
            _model->initNationalRail();

            connect(&_soapHttp, qOverload<const QtSoapMessage &>(&QtSoapHttpTransport::responseReady), this, &TrackerGlue::onNationalRailResponse);

            QTimer::singleShot(50, this, &TrackerGlue::onInitialise );
        }
    };

    ui->frameBuffer->isViewReady([this,startInitialise](bool isReady) {

        if( isReady)
        {
            startInitialise();
        }

        connect(ui->frameBuffer, &TFLViewFrameBuffer::rendererCreated, this, startInitialise);
    });
}

TFLLine* TrackerGlue::getTFLLine(QString id, bool rebuild)
{
    auto it = _rootLines.find(id);

    if( it == _rootLines.end() || rebuild)
    {
#ifdef Q_OS_WIN
        if( it != _rootLines.end())
            qDebug() << "Line : " << id << ": is going to be REBUILT";
#endif
        return buildTFLLine(id).get();

    }

    return it.value().get();
}

bool TrackerGlue::doesBusRouteExist(QString id) const
{
    return _allBusRoutesList.find(id) != _allBusRoutesList.end();
}


void TrackerGlue::buildBusList()
{
    for(auto& item : ui->lineGroupBusList)
    {
        Menu* m = item.value<Menu*>();
        m->deleteLater();
    }

    ui->lineGroupBusList.clear();

    std::map<QString, std::vector<Action*>> categories;

    ui->_busList.removeDuplicates();

      auto& list = ui->_busList;

    std::sort(list.begin(), list.end(), busNumberComparerId);

    for( auto& id : ui->_busList)
    {
        auto line = getTFLLine(id);

        Action* action = new Action(ui);
        action->setObjectName(id);
        action->setText(id);
        action->setData(QVariant::fromValue(line));

        if( id.toInt() >= 600 && id.toInt() < 700)
            categories[QStringLiteral("School")].push_back(action);
        else
            categories[QString(id[0]).toUpper()].push_back(action);
    }

    for(auto& category : categories)
    {
        Menu* m = new Menu(this);
        m->setTitle(category.first);

        for(auto& item : category.second)
            m->addAction(item);

        ui->lineGroupBusList << QVariant::fromValue(m);
    }
}

void TrackerGlue::rebuildBuses(QVariant ids)
{
    QStringList list = ids.toStringList();
    for( const QString &id : std::as_const(list))
        getTFLLine(id, true);
}

int TrackerGlue::getFetchListCount() const
{
    return static_cast<int>(TFLLine::getFullVisibleList().size());
}

int TrackerGlue::getFetchListLimit() const
{
    if( _inAppStore != nullptr && _inAppStore->isAnas())
        return 71;

    return 20;
}

QStringList TrackerGlue::getLineNamesForStopId(QString id) const
{
    QSet<QString> list = _model->getLineNamesForStationId(id);

    for( auto it = list.constBegin(); it != list.constEnd();)
    {
        if( _allRoutesNamesList.constFind(*it) == _allRoutesNamesList.constEnd())
            it = list.erase(it);
        else
            ++it;
    }

    QStringList sl(list.begin(), list.end());
    sl.sort();
    return sl;
}

QStringList TrackerGlue::getLineIDsforStopId(QString id) const
{
    QSet<QString> list = _model->getLineIDsForStationId(id);

    for( auto it = list.constBegin(); it != list.constEnd();)
    {
        if( _allRoutesList.constFind(*it) == _allRoutesList.constEnd())
            it = list.erase(it);
        else
            ++it;
    }

    QStringList sl(list.begin(), list.end());
    sl.sort();
    return sl;
}

void TrackerGlue::updateArrivalsForStopPoint(QString id, QStringList lineWanted)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QString urlText;

    if( lineWanted.length() == 0)
    {
        urlText = _stopPointArrivalsURL.arg(id);
    }
    else
    {
        QString lines = std::accumulate(lineWanted.begin(), lineWanted.end(), QString(""), [](const QString& l, const QString &r) {
            return l + r + ",";
        });

        lines = lines.left(lines.length()-1);

        urlText = _stopPointLineArrivalsURL.arg(lines, id);
    }

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Update Arrivals: " << urlText << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        reply->deleteLater();

        QVariantMap map;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif

            map["InternetError"] = "Check your internet connection (" + QString::number(reply->error()) + ")";
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray array = document.array();

            map = parseJSONArrivals(array);
        }

        emit arrivalsUpdated(map);
    });
}

void TrackerGlue::updateLineModeStatusResults(QString mode)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QString urlText = _lineModeStatusURL.arg(mode);

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Update Tube Status: " << urlText << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        reply->deleteLater();

        QVariantList list;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray array = document.array();
            for(const QJsonValue &value: std::as_const(array))
                list << value;
        }

        emit tubeStatusUpdated(list);
    });
}

void TrackerGlue::updateTubeDisruptionResults()
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QString urlText = _tubeDisruptionURL.arg(QLatin1String("tube,overground,dlr,elizabeth-line,tram"));

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Update Tube Disruption: " << urlText << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        reply->deleteLater();

        QVariantList list;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray array = document.array();
            for(const QJsonValue &value: array)
                list << value;
        }

        emit tubeDisruptionUpdated(list);
    });
}

void TrackerGlue::updateDisruptionForStopPoint(QString id)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QString urlText = _stopPointDisruptionURL.arg(id);

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Update Distruption for StopPoint: " << urlText << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        reply->deleteLater();

        QVariantList list;
        QVariantMap map;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
            map["InternetError"] = "Check your internet connection (" + QString::number(reply->error()) + ")";
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray array = document.array();
            for(const QJsonValue &value: array)
                list << value;

            map["results"] = list;
        }

        emit stopPointDisruptionUpdated(map);
    });
}

void TrackerGlue::updateStatusForLines(QString mode, QStringList lines)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    if( lines.length() > 0)
    {
        QString ids = std::accumulate(lines.begin(), lines.end(), QString(""), [](const QString& l, const QString &r) {
            return l +r + ",";
        });

        query.addQueryItem("ids", ids);
    }

    QString urlText = _lineStatusURL.arg(mode);

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Update Tube Status: " << url.toString()  << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        reply->deleteLater();

        QVariantList list;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray jsonArray = document.array();

            std::vector<QJsonObject> jsonItems;

            for(const QJsonValue &value: jsonArray)
                jsonItems.push_back(value.toObject());

            std::sort(jsonItems.begin(), jsonItems.end(), busNumberComparer);

            for(const auto &item : jsonItems)
                list << item;
        }

        emit lineStatusUpdate(list);
    });
}

void TrackerGlue::updateNationalRailArrivalsForStopPoint(QString id)
{
    QtSoapMessage request;

    QString payload = QString(R"(<?xml version="1.0" encoding="UTF-8"?>
                           <soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/"
                            xmlns:typ="http://thalesgroup.com/RTTI/2013-11-28/Token/types"
                            xmlns:ldb="http://thalesgroup.com/RTTI/2016-02-16/ldb/">
                             <soap:Header>
                               <typ:AccessToken>
                                 <typ:TokenValue>182af13e-cb89-4040-afd5-cdb85e7732de</typ:TokenValue>
                               </typ:AccessToken>
                             </soap:Header>
                             <soap:Body>
                               <ldb:GetDepBoardWithDetailsRequest>
                                 <ldb:crs>%1</ldb:crs>
                               </ldb:GetDepBoardWithDetailsRequest>
                             </soap:Body>
                           </soap:Envelope>)").arg(id);

    request.setContent(payload.toLatin1());

    _soapHttp.setHost("lite.realtime.nationalrail.co.uk", true, 443);
    _soapHttp.submitRequest(request,"/OpenLDBWS/ldb9.asmx");

}

void TrackerGlue::updateVehicleArrivalInfo(QString lineId, QString vehicleId, QString stationName)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QUrl url(_vehicleArrivalURL.arg(vehicleId));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Vehicle Arrival Info: " << vehicleId << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, lineId, stationName, this]
    {
        reply->deleteLater();

        QJsonObject obj;

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif

            obj["InternetError"] = "Check your internet connection (" + QString::number(reply->error()) + ")";
        }
        else
        {
            QByteArray result = reply->readAll();
            QJsonDocument document = QJsonDocument::fromJson(result);
            QJsonArray array = document.array();

            obj = parseJSONVehicle(lineId, stationName, array);
        }

        emit vehicleArrivalInfoUpdated(obj);
    });
}

QVariantMap TrackerGlue::parseJSONArrivals(QJsonArray array)
{
    std::map<QString, std::map<int, QVector<QJsonValue>>> items;

    for(const QJsonValue &value: array)
    {
        QJsonObject obj = value.toObject();

        QString modeName = obj[QStringLiteral("modeName")].toString();
        bool isBus = modeName == QStringLiteral("bus");
        bool isDlr = modeName == QStringLiteral("dlr");

        QJsonValue timeToStation = obj["timeToStation"];

        int timeLeft = static_cast<int>(timeToStation.toInt()/60.0);

        QString lineId = obj["lineId"].toString();

        QString str;
        int dueTime = 1;

        if( lineId == QStringLiteral("jubilee"))
            dueTime = 2;

        if( timeLeft == 0)
            str = "here";
        else if( timeLeft <= dueTime )
            str = "due";
        else
            str = QString("%1mins").arg(QString::number(timeLeft));

        obj["timeToStation"] = QJsonValue(str);

        QString destinationName;

        if( !isBus)
            destinationName = obj["towards"].toString();

        if( destinationName.isEmpty())
            destinationName = obj["destinationName"].toString();

        if( isDlr)
            cleanStationName(destinationName);

        obj["finalDestination"] = QJsonValue(destinationName);

        QString vehicleId = obj["vehicleId"].toString();

        obj["vehicleId"] = QJsonValue(vehicleId);

        QString platformName = obj["platformName"].toString();

        if( isBus)
            platformName = "Bus stop :" + platformName+":";

        items[platformName][timeToStation.toInt()] << obj;
    }

    QVariantMap map;
    for(const auto& item: items)
    {
        QVariantList list;
        for(const auto& jsonVector : item.second)
            for(const auto& json : jsonVector.second)
            {
                bool isBus = json[QStringLiteral("modeName")].toString() ==QStringLiteral("bus");

                if( !isBus)
                    if( list.length() >= 4)
                        break;

                list << json;
            }

         map[item.first] = list;
    }

    return map;
}

QJsonObject TrackerGlue::parseJSONVehicle(QString lineId, QString stationName, QJsonArray array)
{
    QString destinationName;
    QString vehicleId;
    QString lineName;

    QJsonArray stopPoints;

    for(const QJsonValue &value: array)
    {
        QJsonObject obj = value.toObject();

        if( obj["lineId"].toString() != lineId)
            continue;

        if(destinationName.isEmpty())
        {
            destinationName = obj["destinationName"].toString();
        }

        if( vehicleId.isEmpty())
        {
            vehicleId = obj["vehicleId"].toString();
        }

        if( lineName.isEmpty())
        {
            lineName = obj["lineName"].toString();
        }

        QJsonObject objService;
        objService["stationName"] = obj["stationName"].toString();
        objService["platformName"] = obj["platformName"].toString();

        const int timeInMins = obj["timeToStation"].toInt()/60;

        if( timeInMins == 0 )
        {
            objService["timeToStation"] = "due";
        }
        else
        {
            objService["timeToStation"] = QString("%1 mins").arg(timeInMins);
        }

        objService["towards"] = obj["towards"].toString();
        objService["currentLocation"] = obj["currentLocation"].toString();


        const QString dateTimeString = obj["expectedArrival"].toString();
        QDateTime dateTime = QDateTime::fromString(dateTimeString, Qt::ISODateWithMs);
        dateTime.setTimeSpec(Qt::UTC);
        dateTime = dateTime.toLocalTime();
        objService["expectedArrival"] = dateTime.toString("hh:mm");

        stopPoints.append(objService);
    }

    QJsonObject topLevel;

    topLevel["lineId"] = lineId;
    topLevel["lineName"] = lineName;
    topLevel["destinationName"] = destinationName;
    topLevel["vehicleId"] = vehicleId;
    topLevel["stopPoints"] = stopPoints;

    return topLevel;
}

void TrackerGlue::updateCache()
{
    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
        view->updateCache();
    });
}

void TrackerGlue::onZoomIn()
{
    //_RadarModel.radarBlip(0)->aircraft->setBeacon(Places::_EGLL09L);
}

void TrackerGlue::onZoomOut()
{
    //_RadarModel.radarBlip(0)->aircraft->setBeacon(Places::_OCK);
}

void TrackerGlue::onWantGPSUpdate()
{
    if( _locationInfo == nullptr)
        setupGPS();

    if( _locationInfo != nullptr)
    {
        ui->frameBuffer->executeOnRenderThread([](TFLView* view) {

            //float pm = ui->radarView->getPixelsPerMile();
            GPSLocation loc = view->myLocation();
            if( loc != GPSLocation())
            {
                loc._height = view->gpsOrigin()._height;
                view->setGPSOrigin(loc);
            }
        });


#ifdef Q_OS_ANDROID
        if( ui->frameBuffer->getTFLView()->isRealTimeGPS())
            _locationInfo->setPreferredPositioningMethods(QGeoPositionInfoSource::SatellitePositioningMethods);
        else
#endif
            _locationInfo->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);

        _locationInfo->startUpdates();
    }
}

void TrackerGlue::setupGPS()
{
    // obtain the location data source

    auto doInit = [this] {

        if( _locationInfo != nullptr)
            return;

        _locationInfo = QGeoPositionInfoSource::createDefaultSource(
#ifdef Q_OS_WIN
            0
#else
            this
#endif
            );

        if( _locationInfo==nullptr)
            return;

        ui->setGPSSource(_locationInfo);

        int msUpdate = _locationInfo->minimumUpdateInterval();

        qDebug() << "GPS Update Interval : " << msUpdate;

        // select positioning method
        _locationInfo->setPreferredPositioningMethods(QGeoPositionInfoSource::AllPositioningMethods);

        // when the position has changed the setGPSLocation slot is called
        QObject::connect(_locationInfo, &QGeoPositionInfoSource::positionUpdated, this, &TrackerGlue::onGPSUpdated);

        QObject::connect(_locationInfo, &QGeoPositionInfoSource::errorOccurred, this, [this](QGeoPositionInfoSource::Error positioningError){

            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->setMyLocation(view->gpsOrigin());
            });
        });

        onGPSUpdated(_locationInfo->lastKnownPosition());
    };

#ifdef Q_OS_WIN
    doInit();
#else
    QLocationPermission locationPermission;
    locationPermission.setAccuracy(QLocationPermission::Precise);
    if( qApp->checkPermission(locationPermission) == Qt::PermissionStatus::Granted)
    {
        doInit();
        return;
    }

    qApp->requestPermission(locationPermission, doInit);
#endif
}

void TrackerGlue::write_log_file(QString lineId, const QByteArray& json)
{
#ifdef Q_OS_WIN
    time_t t = time(nullptr);
    struct tm newtime ;
    localtime_s(&newtime, &t);

    char logName[16] = "log";
    char name[256] = {};
    char dir[256] = {};
    char postdir[256] = {};

    QDir qtDir(QDir::currentPath());
    qtDir.mkdir(logName);

    sprintf(dir, "%s\\%02d-%02d-%02d",
        logName,
        newtime.tm_mday,
        (newtime.tm_mon + 1),
        newtime.tm_year + 1900 - 2000);

    qtDir.mkdir(dir);

    sprintf(postdir, "%s\\%02d", dir, newtime.tm_hour);

    qtDir.mkdir(postdir);

    sprintf(name, "%s\\%02d-%02d-%02d_%s.json", postdir,
        newtime.tm_hour,
        newtime.tm_min,
        newtime.tm_sec,
            lineId.toLocal8Bit().data());

    std::ofstream outFile(name);

    if (outFile.is_open())
    {
        outFile.write(json, json.length());
        outFile.close();
    }
#endif
}

void TrackerGlue::onGPSUpdated(const QGeoPositionInfo& geoPositionInfo)
{
    if (geoPositionInfo.isValid())
    {
        if( !ui->frameBuffer->getTFLView()->isRealTimeGPS())
            _locationInfo->stopUpdates();

        // get the current location coordinates
        QGeoCoordinate geoCoordinate = geoPositionInfo.coordinate();

        if( ui->frameBuffer->getTFLView()->isRealTimeGPS() )
        {
            RealTimeGPSNewInfo info;

            info.alt = geoCoordinate.altitude();
            if( std::isnan(info.alt))
                info.alt = 0.0f;

            info.hasSpeed = geoPositionInfo.hasAttribute(QGeoPositionInfo::GroundSpeed);
            info.spd = geoPositionInfo.attribute(QGeoPositionInfo::GroundSpeed);

            info.hasHdg = geoPositionInfo.hasAttribute(QGeoPositionInfo::Direction);
            info.hdg = geoPositionInfo.attribute(QGeoPositionInfo::Direction);

            info.hasVSI = geoPositionInfo.hasAttribute(QGeoPositionInfo::VerticalSpeed);
            info.vsi = geoPositionInfo.attribute(QGeoPositionInfo::VerticalSpeed);

            info.lastUpdate = QTime::currentTime();

            ui->frameBuffer->executeOnRenderThread([info](TFLView* view) {
                view->setRealTimeGPSInfo(info);
            });
        }

        GPSLocation location;
        // transform coordinates to lat/lon
        location._lat = geoCoordinate.latitude();
        location._lng = geoCoordinate.longitude();
        location._height = geoCoordinate.altitude();

        if( std::isnan(location._height))
            location._height = 0.0f;

        ui->frameBuffer->executeOnRenderThread([location](TFLView* view) {

            view->setMyLocation(location);

            if( view->isRealTimeGPS())
                view->setGPSOrigin(location);
            else
            {
                GPSLocation newOriginGPS = view->gpsOrigin();
                newOriginGPS._lat = location._lat;
                newOriginGPS._lng = location._lng;
                view->setGPSOrigin(newOriginGPS);
            }
        });
    }
}

void TrackerGlue::onNationalRailResponse(const QtSoapMessage &message)
{
    if (message.isFault())
    {
        qDebug("Error: %s", qPrintable(message.faultString().toString()));
    }
    else
    {
        const QtSoapType &response = message.returnValue();

        QJsonObject jsonPayload;

        jsonPayload["generatedAt"] = response["generatedAt"].value().toString();
        jsonPayload["locationName"] = response["locationName"].value().toString();
        jsonPayload["CRC"] = response["CRC"].value().toString();
        jsonPayload["platformAvailable"] = response["platformAvailable"].value().toBool();

        const QtSoapType &trainServices = response["trainServices"];

        QJsonArray jsonArrServices;

        for(int i=0; i < trainServices.count(); ++i)
        {
            const QtSoapType& service = trainServices[i];

            if(response["locationName"].value().toString() == service["destination"]["location"]["locationName"].value().toString())
                continue;

            QJsonObject objService;
            objService["sta"] = service["sta"].value().toString();
            objService["eta"] = service["eta"].value().toString();
            objService["std"] = service["std"].value().toString();
            objService["etd"] = service["etd"].value().toString();
            objService["platform"] = service["platform"].value().toString();
            objService["operator"] = service["operator"].value().toString();
            objService["operatorCode"] = service["operatorCode"].value().toString();
            objService["serviceType"] = service["serviceType"].value().toString();
            objService["serviceID"] = service["serviceID"].value().toString();
            objService["length"] = service["length"].value().toString();
            objService["fromLocationName"] = service["origin"]["location"]["locationName"].value().toString();
            objService["toLocationName"] = service["destination"]["location"]["locationName"].value().toString();
            objService["via"] = service["destination"]["location"]["via"].value().toString();

            QJsonArray jsonPrevCallPointsArray;
            const QtSoapType& previousCallPoints = service["previousCallingPoints"]["callingPointList"];

            for(int j=0; j < previousCallPoints.count(); ++j)
            {
                const QtSoapType& point = previousCallPoints[j];

                QJsonObject obj;
                obj["locationName"] = point["locationName"].value().toString();
                obj["crs"] = point["crs"].value().toString();
                obj["st"] = point["st"].value().toString();
                obj["at"] = point["at"].value().toString();
                obj["via"] = point["via"].value().toString();

                jsonPrevCallPointsArray.push_back(obj);
            }

            QJsonArray jsonSubseqCallPointsArray;
            const QtSoapType& subsequentCallPoints = service["subsequentCallingPoints"]["callingPointList"];

            for(int j=0; j < subsequentCallPoints.count(); ++j)
            {
                const QtSoapType& point = subsequentCallPoints[j];
                QJsonObject obj;
                obj["locationName"] = point["locationName"].value().toString();
                obj["crs"] = point["crs"].value().toString();
                obj["st"] = point["st"].value().toString();
                obj["et"] = point["et"].value().toString();
                obj["via"] = point["via"].value().toString();

                jsonSubseqCallPointsArray.push_back(obj);
            }

            objService["previousCallingPoints"] = jsonPrevCallPointsArray;
            objService["subsequentCallingPoints"] = jsonSubseqCallPointsArray;

            jsonArrServices.push_back(objService);
        }

        jsonPayload["trainServices"] = jsonArrServices;

        emit nationalRailUpdate(jsonPayload);
    }
}

void TrackerGlue::setFeedingOnline(bool bAllowed)
{
    _bAppFeedingLive = bAllowed;

    ui->frameBuffer->executeOnRenderThread([bAllowed](TFLView* view) {

        view->showSubscribeButton(!bAllowed);

        if(!bAllowed)
        {
            view->setHttpLastErrMsg(QStringLiteral("See [Purchase Options]"));
            view->removeAllVehicles();
        }
    });
}

void TrackerGlue::setUserUIActive(bool isActive)
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO << ":" << isActive;
#endif

    _bUserUIMode = isActive;

    if( ui->frameBuffer != nullptr)
    {
        ui->frameBuffer->executeOnRenderThread([isActive](TFLView* view){
            view->setBlockRendering(isActive);
        });
    }

    if( isActive )
    {
        _perSecondTimer->stop();

        if(_stompNationalRail)
        {
            _stompNationalRail->pause();
        }
    }
    else
    {
        _perSecondTimer->start();
        if(_stompNationalRail)
        {
            _stompNationalRail->resume();
        }
    }
}

void TrackerGlue::initFilterLabelsMenus()
{

}

void TrackerGlue::initUnitsMenus()
{

    ui->action_Units_Spd_Km_H->setData(QVariant::fromValue(Units::Speed::Kilometers));
    ui->action_Units_Spd_Knots->setData(QVariant::fromValue(Units::Speed::Knots));
    ui->action_Units_Spd_Meters_Second->setData(QVariant::fromValue(Units::Speed::Meters));
    ui->action_Units_Spd_MPH->setData(QVariant::fromValue(Units::Speed::Miles));

    connect(ui->menuUnitsSpeed, &ActionGroup::triggered, this, [this](Action* action)
    {
        Units::Speed spd = action->data().value<Units::Speed>();
        ui->frameBuffer->executeOnRenderThread([spd](TFLView* view) {
            view->getUnits().setSpeed(spd);
        });
        saveSettingsInFuture();
    });

    ui->action_Units_Dist_KM->setData(QVariant::fromValue(Units::Distance::Kilometers));
    ui->action_Units_Dist_Meters->setData(QVariant::fromValue(Units::Distance::Meters));
    ui->action_Units_Dist_SM->setData(QVariant::fromValue(Units::Distance::StatueMiles));

    connect(ui->menuUnitsDistance, &ActionGroup::triggered, this, [this](Action* action)
    {
       Units::Distance dst = action->data().value<Units::Distance>();
       ui->frameBuffer->executeOnRenderThread([dst](TFLView* view) {
            view->getUnits().setDistance(dst);
       });
       saveSettingsInFuture();
    });

    ui->action_Units_Alt_Feet->setData(QVariant::fromValue(Units::Altitude::Feet));
    ui->action_Units_Alt_Meters->setData(QVariant::fromValue(Units::Altitude::Meters));
    ui->action_Units_Alt_Nautical_Miles->setData(QVariant::fromValue(Units::Altitude::NauticalMiles));
    ui->action_Units_Alt_Statue_Miles->setData(QVariant::fromValue(Units::Altitude::StatueMiles));
    ui->action_Units_Alt_Kilometers->setData(QVariant::fromValue(Units::Altitude::Kilometers));

    connect( ui->menuUnitsAltitude, &ActionGroup::triggered, this, [this](Action* action)
    {
        Units::Altitude alt = action->data().value<Units::Altitude>();
        ui->frameBuffer->executeOnRenderThread([alt](TFLView* view) {
            view->getUnits().setAltitude(alt);
       });
       saveSettingsInFuture();
    });


        ui->action_Units_VSI_Minute->setData(QVariant::fromValue(Units::PerInterval::Minute));
    ui->action_Units_VSI_Second->setData(QVariant::fromValue(Units::PerInterval::Second));

    connect( ui->menuUnitsVsiInterval, &ActionGroup::triggered, this, [this](Action* action)
    {
       Units::PerInterval perInterval = action->data().value<Units::PerInterval>();
       ui->frameBuffer->executeOnRenderThread([perInterval](TFLView* view) {
            view->getUnits().setVsiPerInteraval(perInterval);
       });
       saveSettingsInFuture();
    });

    ui->action_Units_Alt_Feet->trigger();
    ui->action_Units_Dist_Meters->trigger();
    ui->action_Units_Spd_MPH->trigger();
    ui->action_Units_VSI_Minute->trigger();
}

void TrackerGlue::onInitialise()
{
    if( _bInitialised)
        return;

    _bInitialised = true;

    ui->setupUi();
    ui->frameBuffer->window()->installEventFilter(this);

    connectOSM();

    connect(this, &TrackerGlue::allLinesAdded, this, [this](QVector<QString> ids) {

        for(const QString& id: std::as_const(ids))
            getTFLLine(id,true);

        setUpFetchList();

        setupFetchTimer();

#ifdef Q_OS_WIN
        checkNationalRailStns();
#endif
        ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
            view->updateCache();
            view->setReady(true);
        });

        emit completed();
    });

    QSettings s;

    const QStringList allRouteList = s.value("Settings/allRoutesList").toStringList();
    _allRoutesList = QSet<QString>(allRouteList.begin(), allRouteList.end());

    loadAllRoutes();

    loadOSMData();

    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
        view->setGPSLimitBoundary({GPSLocation(58, -9), GPSLocation(50,3)}, true);
    });

    connect(ui->action_Exit, &Action::triggered, this, &TrackerGlue::close );

    _trafficAlert = new QSoundEffect( this);
    _trafficAlert->setSource(QUrl::fromLocalFile(ResourcePath("sounds/trafficalert.wav")));
    _trafficAlert->setLoopCount(1);

    _timerTrafficAlert = new QTimer(this);
    _timerTrafficAlert->setObjectName("FlightTracker::timerTrafficAlert");

    _perSecondTimer = new QTimer(this);
    _perSecondTimer->setObjectName("perSecondTimer");

    _compass = new QCompass(this);

    initBlipVerbosityMenus();
    initUnitsMenus();
    initFilterLabelsMenus();

    connect(ui->action_ZoomIn, SIGNAL(triggered()), SLOT(onZoomIn()));
    connect(ui->action_ZoomOut, SIGNAL(triggered()), SLOT(onZoomOut()));

   // ui->tflView->connect(ui->action_ZoomIn, SIGNAL(triggered()), SLOT(onZoomIn()));
   // ui->tflView->connect(ui->action_ZoomOut, SIGNAL(triggered()), SLOT(onZoomOut()));

#ifdef __IGNORE__

//    connect(ui->tflView, &TFLView::onSwitchView3D, this, [this](bool bIs3D)
//    {
//        for(auto& a:ui->menu_3D->actions())
//            a->setEnabled(bIs3D);
//    });

//    ui->tflView->installEventFilter(this);
//    ui->transmitLineEdit->installEventFilter(this);

#endif

    connect(_compass, &QCompass::readingChanged, this, [this] {

        if( _bUserUIMode )
            return;

        if( !ui->action_Compass->isChecked())
            return;

        float fCompassReading = DegreesToRadians( _compass->reading()->azimuth());

        _compassMeasurements[_compassIdx].x = sin( fCompassReading);
        _compassMeasurements[_compassIdx].y = cos( fCompassReading);
        if( ++_compassIdx == COMPASS_MEASUREMENTS)
            _compassIdx = 0;

        Vector3F avValue = std::accumulate(_compassMeasurements, _compassMeasurements+COMPASS_MEASUREMENTS, Vector3F());

        float fAngle = 90-RadiansToDegrees( std::atan2(avValue.y, avValue.x));

        ui->frameBuffer->executeOnRenderThread([fAngle](TFLView* view) {
            view->setCompassValue( fAngle, true );
        });
    });

//    ui->statusBar->hide();

//#ifdef Q_OS_ANDROID
    ui->menu_View->removeAction(ui->action_Debug);
    ui->menu_Mode->menuAction()->setVisible(false);
    ui->action_Close_Simulation->setVisible(false);
    ui->action_Pause->setVisible(false);

    ui->actionIsPurchased->setVisible(false);
    ui->actionPurchase->setVisible(false);
    ui->action_android_test_canceled->setVisible(false);
    ui->action_android_test_item_unavailable->setVisible(false);
    ui->action_android_test_purchased->setVisible(false);
    ui->action_android_test_refunded->setVisible(false);
//#endif

    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantToGo, this, [this](GPSLocation location) {
        location._height = ui->frameBuffer->getTFLView()->gpsOrigin()._height;

        ui->frameBuffer->executeOnRenderThread([location](TFLView* view) {
            view->setGPSOrigin(location, -1, true, true );
        });
    });

    connect(ui->frameBuffer, &TFLViewFrameBuffer::onWantGPSUpdate, this, &TrackerGlue::onWantGPSUpdate, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::onWantRealTimeGPS, ui->action_RealTime_GPS, &Action::setChecked, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::onWantCompassMode, this, [this]
    {
        if( ui->action_RealTime_GPS->isChecked())
        {
            ui->action_RealTime_GPS->setChecked(false);

            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->setCompassValue(0.0, true);
            });
        }
        else
            ui->action_Compass->toggle();

    }, Qt::QueuedConnection);


    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantVR, ui->action_3D_VR, &Action::trigger, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantShow3D, this, [this](bool b3D)
            {
                ui->frameBuffer->executeOnRenderThread([b3D](TFLView* view) {
                    view->setView3D(b3D);
                });

                saveSettingsInFuture();
            }, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::onDisplayInitialised, this, &TrackerGlue::onDisplayInitialised);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantToggleColor, this, [this]
            {
                if( _bUserUIMode)
                    return;

                if( ui->frameBuffer->getTFLView()->isView3D())
                {
                    ui->actionGroup_skyLineGroup->triggerNext();
                }
                else
                {
                    ui->action_2D_Map_Night->toggle();
                }

                ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                    view->updateCache();
                });
                saveSettingsInFuture();
            }, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantMuteProximity, this, [this](bool bMute)
            {
                ui->action_Proximity_Mute_sound->setChecked(bMute);
                saveSettingsInFuture();
            }, Qt::QueuedConnection);

    connect(ui->frameBuffer, &TFLViewFrameBuffer::wantToSubscribe, this, &TrackerGlue::showPurchasePage);

    auto updateRealTimeMode = [this] {

        TFLView::RealTimeMode rtm;

        if( ui->action_Compass->isChecked())
            rtm = TFLView::RealTimeMode::Compass;

        else if( ui->action_RealTime_GPS->isChecked())
            rtm = TFLView::RealTimeMode::GPS;

        else
            rtm = TFLView::RealTimeMode::None;

        ui->frameBuffer->executeOnRenderThread([rtm](TFLView* view) {
            view->setRealTimeMode(rtm);
        });

    };

    connect(ui->action_RealTime_GPS, &Action::toggled, this, [this,update=updateRealTimeMode](bool checked) {

        if( checked)
        {
            ui->action_Compass->setChecked(false);

            const int updateInterval = _locationInfo == nullptr ? 1000 : _locationInfo->updateInterval();
            ui->frameBuffer->executeOnRenderThread([updateInterval](TFLView* view) {
                view->setGPSUpdateInterval(updateInterval);
            });
            onWantGPSUpdate();
        }

        update();
        ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
            view->updateCache();
        });
    });

    connect(ui->action_Compass, &Action::toggled, this, [this,update=updateRealTimeMode](bool checked) {
        if( checked)
        {
            ui->action_RealTime_GPS->setChecked(false);
            _compass->start();
        }
        else
        {
            _compass->stop();

            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->setCompassValue(0.0, true);
            });
        }

        update();
    });

    connect( ui->action_TFL_BusStop_Visible, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setBusStopVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect( ui->action_TFL_BusLine_Visible, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setBusLinesVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect( ui->action_TFL_TubeLine_Visible, &Action::toggled, this, [this](bool checked)
    {
       ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setTubeLinesVisible(checked);
       });
       saveSettingsInFuture();
    });

    connect(ui->action_2D_Map_Night, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setMapNight(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_3D_Show_You, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->get3D()->setShowYou(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_piccadillyNormalDestination, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setPiccHeathrowDestnNormalFormat(checked);
        });
       saveSettingsInFuture();
    });

    connect(ui->action_Use_Vehicle_Behaviour, &Action::toggled, this, [this](bool checked)
    {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setUseVehicleBehaviour(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_Elizabeth_Arrivals_Use_TFL_Data, &Action::toggled, this, [this](bool checked)
            {
                _elizabethLineUsesTFL = checked;

                ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                    view->removeVehicles(Line::elizabethLine);
                });
                saveSettingsInFuture();
            });

    for(const QString& id : ui->frameBuffer->getTFLView()->get3D()->getSkyBoxIds())
    {
        Action* a = new Action(id, ui);
        a->setCheckable(true);
        a->setObjectName("action_3D_Sky_Line_" + id);
        a->setData(id);
        a->setChecked(id == "TropicalSunnyDay");

        ui->actionGroup_skyLineGroup->addAction(a);
    }

    connect(ui->actionGroup_skyLineGroup, &ActionGroup::triggered, this, [this](Action* a)
    {
        const QString skyId = a->data().toString();
        ui->frameBuffer->executeOnRenderThread([skyId](TFLView* view) {
            view->get3D()->setSkyLineId(skyId);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_3D_VR, &Action::triggered, ui->frameBuffer, &TFLViewFrameBuffer::activate3DVR);

    connect(ui->action_GPS, &Action::triggered, this, [this] {
        onWantGPSUpdate();
    });

    connect(ui->action_Show_Proximity_Rings, &Action::toggled, this, [this](bool checked) {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->setShowProximityRings(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_Proximity_Mute_sound, &Action::toggled, this, [this](bool checked) {

        const bool proxActive = ui->action_Proximity_Active->isChecked();
        ui->frameBuffer->executeOnRenderThread([proxActive, checked](TFLView* view) {
            view->setProximityActive(proxActive, !checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_Proximity_Active, &Action::toggled, this, [this](bool checked) {

        const bool proxSound = ui->action_Proximity_Mute_sound->isChecked();
        ui->frameBuffer->executeOnRenderThread([proxSound, checked](TFLView* view) {
            view->setProximityActive(checked, !proxSound);
        });
        saveSettingsInFuture();
    });

    std::vector<QString> ids = {
        "200", "201", "202", "203", "204", "205", "206",
        "207", "210", "211", "212", "213", "214", "215",
        "216", "217", "175", "172"
    };

    for(const auto &id : ids)
    {
        Action* a = ui->menu_Circle_ID_Color_Override->addAction(id);
        a->setText(id);
        a->setData(id);
        a->setObjectName("action_Circle_ID_Override_" + id);
        a->setCheckable(true);

        connect(a, &Action::toggled, this, [this,a](bool checked) {

            const QString id = a->data().toString();
            ui->frameBuffer->executeOnRenderThread([id,checked](TFLView* view) {
                view->setCircleColorOverride(id, checked);
            });
            saveSettingsInFuture();
        }, Qt::QueuedConnection);
    }

    connect(_perSecondTimer, &QTimer::timeout, this, [this] {

        ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
            view->updateTime();
        });

#ifdef Q_OS_WIN
        QString powerLevel;
        bool isCharging = false;

        SYSTEM_POWER_STATUS status;
        GetSystemPowerStatus(&status);

        if( status.BatteryFlag == 255 )
        {
            powerLevel = "???";
        }
        else if( status.BatteryFlag & 128)
        {
            powerLevel = "N/A";
        }
        else
        {
            powerLevel = QString::number(status.BatteryLifePercent);

            if( (status.BatteryFlag & 8) == 0)
                isCharging = false;

            if( status.BatteryLifePercent < 100)
                isCharging = (status.BatteryFlag & 8);
        }

        ui->frameBuffer->executeOnRenderThread([powerLevel, isCharging](TFLView* view) {
            view->setBatteryInfo(powerLevel, isCharging);
        }, false);
#endif

        if( _inAppStore != nullptr)
        {
            std::vector<QString> msgs;
            const auto interval = _perSecondTimer->interval()/1000.0f;

            auto updateTimeLeft = [this,&msgs,&interval](const QString& msg, const QString& purchaseID ){

                const int minutesLeft = _inAppStore->minutesLeft(purchaseID);

                if( minutesLeft < 6)
                {
                    msgs.push_back(QString("%1 mins free").arg(minutesLeft));
                    msgs.push_back(QStringLiteral("See [Purchase Options] menu"));
                }

                _inAppStore->update(purchaseID, interval);
            };

#ifdef __NOT_USING_SUBSCRIPTION__
            if( ui->tflView->getProximityActive() )
                updateTimeLeft("Proximity Alert", InAppStore::AppStoreID_ProximityAlert);

            if(ui->tflView->isRealTimeGPS() && !ui->tflView->hasGPSTimedOut())
                updateTimeLeft("RealTime-GPS", InAppStore::AppStoreID_RealTimeGPS);

            if(ui->tflView->isView3D() )
                updateTimeLeft("3D-View", InAppStore::AppStoreID_EstimatedPosition);

            if( _fetchList.size() > _maxInAppVehicleCount )
                updateTimeLeft("Estimated Position", InAppStore::AppStoreID_EstimatedPosition);
#endif

            if( _inAppStore->isFreeUser())
            {
                //Do nothing.
            }
            else if( _inAppStore->isPurchased(InAppStore::AppStoreID_LifeTime_Purchase))
            {
                //do nothing
            }
            else if( _inAppStore->isPurchased(InAppStore::AppStoreID_Monthly_Subcriber))
            {
                //do nothing
            }
            else if( _inAppStore->isMe() && ui->action_InAppCheckMe->isChecked() == false)
            {
                //its me.
            }
            else if( _bAppFeedingLive)
                updateTimeLeft("Ongoing-Support", InAppStore::AppStoreID_Monthly_Subcriber);

            ui->frameBuffer->executeOnRenderThread([msgs](TFLView* view) {

                view->setShowTopLeftInfo(true);
                view->setInAppEvalMsgs(msgs);
            });
        }

        if( _showReleaseNotesMins >= 0)
        {
            if( !ui->frameBuffer->getTFLView()->isBlockRendering())
            {
                if( _showReleaseNotesMins == 0)
                    emit showReleaseNotes();

                _showReleaseNotesMins--;
            }
        }
    });

    connect( _timerTrafficAlert, &QTimer::timeout, this, [this]
            {
                if( ui->action_Proximity_Mute_sound->isChecked())
                    return;

                if( _trafficAlert->isPlaying())
                    return;

                _trafficAlert->play();

            });

    connect(ui->frameBuffer, &TFLViewFrameBuffer::onProximityWarningActive, this, [this]( bool warningActive) {

        if( warningActive)
        {
            QMetaObject::invokeMethod(_timerTrafficAlert, "timeout", Qt:: QueuedConnection);
            _timerTrafficAlert->start(10000);
        }
        else
            _timerTrafficAlert->stop();
    });

    connect( _mapTilesEntries, &MapTileEntries::activeChanged, this, [this] {

        const bool isActive = _mapTilesEntries->getActive();

        ui->frameBuffer->executeOnRenderThread( [isActive](TFLView* view) {

            view->get3D()->setTileMapActive(isActive);
            view->setTilesDirty();
        });

    });

    connect( _mapTilesEntries, &MapTileEntries::showZoomChanged, this, [this] {

        const bool isShowZoom = _mapTilesEntries->getShowZoom();

        ui->frameBuffer->executeOnRenderThread( [isShowZoom](TFLView* view) {
            view->get3D()->setTileMapShowZoom(isShowZoom);
            view->setTilesDirty();
        });
    });

    connect( _mapTilesEntries, &MapTileEntries::tileUpdate, this, [this] {

        const MapTileEntry* entry = _mapTilesEntries->getActiveEntry();

        if( entry == nullptr)
        {
            ui->frameBuffer->executeOnRenderThread( [](TFLView* view) {

                view->get3D()->setTileMapURLs({"",""});
                view->setTilesDirty();
            });

            return;
        }

        const QStringList urls = {entry->url(), entry->url2()};

        const bool showRunway = entry->showRunway();
        const bool showZoom12 = entry->showZoom12();
        const bool showZoom13 = entry->showZoom13();
        const bool showZoom14 = entry->showZoom14();

        const QString userName = entry->user();
        const QString password = entry->password();


        ui->frameBuffer->executeOnRenderThread( [urls,
                                                userName,
                                                password,
                                                showRunway,
                                                showZoom12,showZoom13,showZoom14](TFLView* view) {

            auto obj = view->get3D();
            obj->setTileMapURLs(urls);
            obj->setTileMapUser(userName);
            obj->setTileMapPassword(password);
            obj->seTileShowRunway(showRunway);
            obj->setTileZoomVisibility(12, showZoom12);
            obj->setTileZoomVisibility(13, showZoom13);
            obj->setTileZoomVisibility(14, showZoom14);

            view->setTilesDirty();

        });
    });

    initReleaseNotes();

    loadTFLModes();
    setupHardcodedColors();

    setGlueReady();
}

QString TrackerGlue::uniqueAndroidID()
{
#ifdef Q_OS_ANDROID
    QJniObject myID = QJniObject::fromString("android_id");
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject appctx = activity.callObjectMethod("getApplicationContext","()Landroid/content/Context;");
    QJniObject contentR = appctx.callObjectMethod("getContentResolver", "()Landroid/content/ContentResolver;");
    QJniObject result = QJniObject::callStaticObjectMethod(
                "android/provider/Settings$Secure",
                "getString",
                "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;"
                ,contentR.object<jobject>(),
                myID.object<jstring>());
    return result.toString();
#else
    return InAppStore::AndroidID_Windows;
#endif
}

void TrackerGlue::showToast(const QString &message, Duration duration)
{

#ifdef Q_OS_ANDROID

    QNativeInterface::QAndroidApplication::runOnAndroidMainThread([message]{
        QJniObject activity = QNativeInterface::QAndroidApplication::context();
        QJniObject javaString = QJniObject::fromString(message);
        activity.callMethod<void>("showToast", "(Ljava/lang/String;)V", javaString.object<jstring>());
    });

#endif
#ifdef Q_OS_WIN
    emit qmlPopUp("showToast", message);
#endif
}

void TrackerGlue::initReleaseNotes()
{
    const char *build_str = "Built: " __DATE__ " " __TIME__;

    QSettings s;
    QString strBuildDateTime = s.value(BuildVersion::BuildDateTime, "").toString();
    s.setValue(BuildVersion::BuildDateTime, QString(build_str));

    if(strBuildDateTime != build_str)
        _showReleaseNotesMins = 10;

#ifdef Q_OS_WIN
    qDebug() << "Previous : " << strBuildDateTime;
    qDebug() << "Compiled : "  << build_str;
#endif
}

void TrackerGlue::initInAppStore()
{
    _inAppProductMap[InAppStore::AppStoreID_3DView] = ui->action_InApp_3DView;
    _inAppProductMap[InAppStore::AppStoreID_RealTimeGPS] = ui->action_InApp_RealTimeGPS;
    _inAppProductMap[InAppStore::AppStoreID_ProximityAlert] = ui->action_InApp_ProximityAlert;
    _inAppProductMap[InAppStore::AppStoreID_EstimatedPosition] = ui->action_InApp_VehiclePosition;
    _inAppProductMap[InAppStore::AppStoreID_Monthly_Subcriber] = ui->action_Purchase_Monthly_Sub;
    _inAppProductMap[InAppStore::AppStoreID_LifeTime_Purchase] = ui->action_Purchase_Lifetime;

    connect(ui->action_InApp_3DView, &Action::triggered, this, [this]
    {
       _inAppStore->purchase(InAppStore::AppStoreID_3DView);
    });

    connect(ui->action_InApp_RealTimeGPS, &Action::triggered, this, [this]
    {
       _inAppStore->purchase(InAppStore::AppStoreID_RealTimeGPS);
    });

    connect(ui->action_InApp_ProximityAlert, &Action::triggered, this, [this]
    {
       _inAppStore->purchase(InAppStore::AppStoreID_ProximityAlert);
    });

    connect(ui->action_InApp_VehiclePosition, &Action::triggered, this, [this]
    {
       _inAppStore->purchase(InAppStore::AppStoreID_EstimatedPosition);
    });

    connect(ui->action_Purchase_Monthly_Sub, &Action::triggered, this, [this]
    {
        if( !_inAppStore->isPurchased(InAppStore::AppStoreID_LifeTime_Purchase))
            _inAppStore->purchase(InAppStore::AppStoreID_Monthly_Subcriber);
    });

    connect(ui->action_Purchase_Lifetime, &Action::triggered, this, [this]
    {
        if( !_inAppStore->isPurchased(InAppStore::AppStoreID_Monthly_Subcriber))
            _inAppStore->purchase(InAppStore::AppStoreID_LifeTime_Purchase);
    });

    _inAppStore = new InAppStore(uniqueAndroidID(), this);

    connect( _inAppStore, &InAppStore::promptProductPurchase, this, &TrackerGlue::qmlPromptProductPurchase);

    auto updateStatus = [this](QString id)
    {
        QString cost = _inAppStore->cost(id);
        bool purchased = _inAppStore->isPurchased(id);
        bool isInapp = _inAppStore->isInApp(id);
        Action* action = _inAppProductMap[id];

        if( action != nullptr)
        {
            if( isInapp )
            {
                if(purchased)
                    action->setText(QString("%1 - (%2)")
                                              .arg(id, "Purchased"));
                else
                    action->setText(QString("%1 - (%2 - %3)").arg(id, "Purchase", cost));
            }
            else
            {
                if(purchased)
                    action->setText(QString("%1 - (%2)").arg(id, "Subscribed"));
                else
                    action->setText(QString("%1 - (%2 - %3)").arg(id, "Subcribe", cost));
            }

            action->setDisabled(purchased);
            action->setVisible( (_inAppStore->isMe() && purchased) || id == InAppStore::AppStoreID_Monthly_Subcriber || id == InAppStore::AppStoreID_LifeTime_Purchase);
        }
    };

    connect( _inAppStore, &InAppStore::allProductsRegistered, this, [this] {

        const int numProds = _inAppStore->numberOfPurchases();
        int secondsWanted = std::max(10*60, numProds * 60*60);

        qDebug() << "Number of Purchases : " << numProds;

        if( _inAppStore->isMe())
            secondsWanted = 30;

        _inAppStore->setEvalTimeInSecs(InAppStore::AppStoreID_Monthly_Subcriber, secondsWanted);
    });

    connect( _inAppStore, &InAppStore::productKnown, this, updateStatus, Qt::QueuedConnection);

    connect( _inAppStore, &InAppStore::promptPurchasePage, this, &TrackerGlue::showPurchasePage);

    connect( _inAppStore, &InAppStore::productPurchased, this, &TrackerGlue::productPurchased);

    connect( _inAppStore, &InAppStore::productPurchased, this, [this,updateStatus](QString id, bool purchased)
    {
        if( id == InAppStore::AppStoreID_3DView)
        {
            ui->frameBuffer->executeOnRenderThread([purchased](TFLView* view) {
                view->setView3D(purchased);
            });
        }
        else if( id == InAppStore::AppStoreID_RealTimeGPS)
        {
            if( purchased != ui->action_RealTime_GPS->isChecked())
                ui->action_RealTime_GPS->toggle();
        }
        else if( id == InAppStore::AppStoreID_ProximityAlert)
        {
            if( purchased != ui->action_Proximity_Active->isChecked())
                    ui->action_Proximity_Active->toggle();
        }
        else if( id == InAppStore::AppStoreID_EstimatedPosition)
        {
            if( !purchased)
            {
                int numberActive(0);
                for( auto it = _fetchList.begin(); it != _fetchList.end() ;++it)
                {
                    auto line = getTFLLine(*it);

                    if( line == nullptr)
                        continue;

                    if( line->isVisible())
                        numberActive++;

                    if( numberActive > _maxInAppVehicleCount )
                        line->setVisible(false);
                }

                updateFetchFilterList();
            }
        }
        else if( id == InAppStore::AppStoreID_Monthly_Subcriber || id == InAppStore::AppStoreID_LifeTime_Purchase)
        {
            setFeedingOnline(purchased);
        }

        updateStatus(id);
    });

    _inAppStore->initialise();
}

void TrackerGlue::triggerOSM()
{
    auto& osm = ui->frameBuffer->getTFLView()->getOSMRenderer();

    ui->action_OSM_Motorway_Visible->setChecked(osm.isMotorwayVisible());
    ui->action_OSM_Primary_Visible->setChecked(osm.isPrimaryVisible());
    ui->action_OSM_Secondary_Visible->setChecked(osm.isSecondaryVisible());
    ui->action_OSM_Tertiary_Visible->setChecked(osm.isTertiaryVisible());
    ui->action_OSM_Residential_Visible->setChecked(osm.isResidentialVisible());
    ui->action_OSM_Footway_Visible->setChecked(osm.isFootwayVisible());
    ui->action_OSM_Water_Visible->setChecked(osm.isWaterVisible());
    ui->action_OSM_Aeroway_Visible->setChecked(osm.isAerowayVisible());
    ui->action_OSM_CycleWay_Visible->setChecked(osm.isCycleWayVisible());
    ui->action_OSM_Pedestrian_Visible->setChecked(osm.isPedestrianVisible());
}

void TrackerGlue::connectOSM()
{
    connect(ui->action_OSM_Motorway_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setMotorwayVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Primary_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setPrimaryVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Secondary_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setSecondaryVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Tertiary_Visible, &Action::toggled, this, [this](bool checked) {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setTertiaryVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Residential_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setResidentialVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Footway_Visible, &Action::toggled, this, [this](bool checked) {
        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setFootwayVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Water_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setWaterVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Aeroway_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setAerowayVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_CycleWay_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setCycleWayVisible(checked);
        });
        saveSettingsInFuture();
    });

    connect(ui->action_OSM_Pedestrian_Visible, &Action::toggled, this, [this](bool checked) {

        ui->frameBuffer->executeOnRenderThread([checked](TFLView* view) {
            view->getOSMRenderer().setPedestrianVisible(checked);
        });
        saveSettingsInFuture();
    });
}

void TrackerGlue::saveSettingsInFuture()
{
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif
    _bWantToSave = true;
}

void TrackerGlue::updateFetchFilterList()
{
    _fetchList.clear();

    const std::set<QString>& ids = TFLLine::getFullVisibleList();

    int count = 0;
    for( auto& id : ids)
    {
        if( count > getFetchListLimit())
            break;

        TFLLine* line = getTFLLine(id);

        if( line->isOffSetDirty())
            line->updateBranches();

        if( line->id() == QStringLiteral("thameslink"))
            _fetchList << id;

        if( line->isNationalRail())
            _fetchNationalRailList << id;
        else
            _fetchList << id;

        count++;
    }

#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif

    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
        view->updateViewBox(true);
    });
    saveSettingsInFuture();
}

void TrackerGlue::updateNationalRailList()
{
    _NationalRailList.clear();
    _fetchNationalRailList.clear();

    std::set<QString> ids = TFLLine::getNationalRailList();
    ids.insert(TFLLine::getRiverBoatList().begin(), TFLLine::getRiverBoatList().end());

    for( auto& id : ids)
    {
        TFLLine* line = getTFLLine(id);

        _NationalRailList << id;;

        if( line->isOffSetDirty())
            line->updateBranches();

        if( line->isVisible())
            _fetchNationalRailList << id;
    }

#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO;
#endif

    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
        view->updateViewBox(true);
    });
    saveSettingsInFuture();
}

void TrackerGlue::initBlipVerbosityMenus()
{
    ui->action_Bus_Verbosity_None->setData(BlipVerbosity::none);
    ui->action_Bus_Verbosity_LineId->setData(BlipVerbosity::LineId);
    ui->action_Bus_Verbosity_VehicleId->setData(BlipVerbosity::VehicleId);
    ui->action_Bus_Verbosity_All->setData(BlipVerbosity::All);

    connect(ui->menu_Label_Bus_Verbosity, &ActionGroup::triggered, this, [this](Action* action) {
        BlipVerbosity verb = action->data().value<BlipVerbosity>();

        ui->frameBuffer->executeOnRenderThread([verb](TFLView* view) {
            view->setBusBlipVerbosity(verb);
        });
        saveSettingsInFuture();
    });
    ui->action_Bus_Verbosity_All->trigger();

    ui->action_Train_Verbosity_None->setData(BlipVerbosity::none);
    ui->action_Train_Verbosity_LineId->setData(BlipVerbosity::LineId);
    ui->action_Train_Verbosity_VehicleId->setData(BlipVerbosity::VehicleId);
    ui->action_Train_Verbosity_All->setData(BlipVerbosity::All);

    connect(ui->menu_Label_Train_Verbosity, &ActionGroup::triggered, this, [this](Action* action) {
        BlipVerbosity verb = action->data().value<BlipVerbosity>();

        ui->frameBuffer->executeOnRenderThread([verb](TFLView* view) {
            view->setTrainBlipVerbosity(verb);
        });
        saveSettingsInFuture();
    });

    ui->action_Train_Verbosity_VehicleId->trigger();

    ui->action_3D_Blip_Verbosity_None->setData(BlipVerbosity::none);
    ui->action_3D_Blip_Verbosity_ID->setData(BlipVerbosity::LineId);
    ui->action_3D_Blip_Verbosity_All->setData(BlipVerbosity::All);

    connect(ui->menu3DBlipVerbosity, &ActionGroup::triggered, this, [this](Action* action) {
        BlipVerbosity verb = action->data().value<BlipVerbosity>();
        ui->frameBuffer->executeOnRenderThread([verb](TFLView* view) {
            view->get3D()->setBlipVerbosity(verb);
        });
        saveSettingsInFuture();
    });

    ui->action_3D_Blip_Verbosity_ID->trigger();
    ui->action_Call_Sign->trigger();
}

std::shared_ptr<Line> TrackerGlue::getLine(const QString &direction, const QString &id)
{
    return _model->getLine(direction, id);
}

void TrackerGlue::addRoute(std::shared_ptr<Line> line, std::shared_ptr<StopPointMins> timeData)
{
    _model->addRoute(line, timeData);
}

std::shared_ptr<TFLLine> TrackerGlue::buildTFLLine(QString line)
{
    std::map<LineType::mode, Menu*> typeMenu;

    typeMenu[LineType::coach] = ui->lineGroup_coach;
    typeMenu[LineType::cycle] = ui->lineGroup_cycle;
    typeMenu[LineType::cycle_hire] = ui->lineGroup_cycle_hire;
    typeMenu[LineType::dlr] = ui->lineGroup_tube;
    typeMenu[LineType::overground] = ui->lineGroup_tube;
    typeMenu[LineType::replacement_bus] = ui->lineGroup_replacement_bus;
    typeMenu[LineType::river_bus] = ui->lineGroup_river_bus;
    typeMenu[LineType::river_tour] = ui->lineGroup_river_tour;
    typeMenu[LineType::taxi] = ui->lineGroup_taxi;
    typeMenu[LineType::elizabeth] = ui->lineGroup_tube;
    typeMenu[LineType::tram] = ui->lineGroup_tube;
    typeMenu[LineType::tube] = ui->lineGroup_tube;
    typeMenu[LineType::national_rail] = ui->lineGroup_national_rail;
    typeMenu[LineType::cable_car] = ui->lineGroup_cable_car;

    std::shared_ptr<Line> outbound = getLine(Line::outbound, line);
    std::shared_ptr<Line> inbound = getLine(Line::inbound, line );

    if( outbound == nullptr)
        return nullptr;

    auto type = outbound->getType();

    auto it = _rootLines.find(line);

    std::shared_ptr<TFLLine> tflLine;

    if( it == _rootLines.end())
    {
        tflLine = std::make_shared<TFLLine>();

        QQmlEngine::setObjectOwnership(tflLine.get(), QQmlEngine::CppOwnership);

        _rootLines[line] = tflLine;

        tflLine->setObjectName(line);
        tflLine->setLines(inbound,outbound);

        int offset = type == LineType::bus ? 10 : 50;

        auto itOffset = _lineOffsetMap.find(line);
        if( itOffset != _lineOffsetMap.end())
            offset = itOffset->second;

        QColor color;

        auto itColor = _lineColorMap.find(line);

        if( itColor != _lineColorMap.end())
        {
            color = itColor->second;
        }
        else
        {
            static QRandomGenerator random;
            color = QColor::fromRgba(qRgb(random.generateDouble()*256, random.generateDouble()*256, random.generateDouble()*256));
        }

        tflLine->setOffSet(offset);
        tflLine->setColor(color);
    }
    else
    {
        tflLine = it.value();

        auto color = tflLine->getColor();
        auto offSet = tflLine->getOffset();
        auto visible = tflLine->isVisible();
        auto showStops = tflLine->getShowStops();
        auto updatedOK = tflLine->getUpdatedOK();
        auto updateDate = tflLine->getUpdateDate();

        tflLine->setLines(inbound,outbound);

        tflLine->setColor(color);
        tflLine->setOffSet(offSet);
        tflLine->setVisible(visible);
        tflLine->setShowStops(showStops);
        tflLine->setUpdatedOK(updatedOK);
        tflLine->setUpdateDate(updateDate);
    }

    if( type == LineType::bus)
        return tflLine;

    if(type ==LineType::cable_car)
    {
        int i;
        i =3;
    }

    Menu* menu = typeMenu[type];
    if( menu == nullptr)
    {
        qDebug() << "Line type not found :" << outbound->id() << "," << outbound->name();
        return tflLine;
    }

    Action* action = new Action(ui);
    action->setObjectName(outbound->id());
    action->setText(outbound->id());
    action->setData(QVariant::fromValue(tflLine.get()));
    menu->addAction(action);

    return tflLine;
}

void TrackerGlue::setupFetchTimer()
{
    _fetchTimer = new QTimer(this);
    _fetchTimer->setObjectName("fetchTimer");

    connect(_fetchTimer, &QTimer::timeout, this, [this]{

        ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
            view->removeOldVehicles();
        });

        if( /*_fetchList.empty() || */_bUserUIMode)
        {
            _fetchTimer->setInterval(1000);
        }
        else if( _currentlyDownloading )
        {
            _fetchTimer->setInterval(500);
        }
        else if( _bAppFeedingLive )
        {
            if( !_allLoadedRoutesRefreshed )
            {
                prepareAllRoutes();
                _fetchTimer->setInterval(500);
            }
            else
            {
                if( _downloadTurn == DownloadTurn::END)
                    _downloadTurn = DownloadTurn::BEGIN;

                ++(int&)(_downloadTurn);

                if( _downloadTurn == DownloadTurn::TFLOrNetworkRail)
                {
                    tflFetch();
                    stompFetch();
                    _fetchTimer->setInterval(30000);
                }
            }
        }
        else
        {
            _fetchTimer->setInterval(1000);
        }
    });

    _fetchTimer->start(1000);
}

void TrackerGlue::setupHardcodedColors()
{
    _lineColorMap[Line::elizabethLine] = "#6950a1";

    _lineColorMap[Line::libertyLine] = "#61686B";
    _lineColorMap[Line::lionessLine] = "#FFA600";
    _lineColorMap[Line::mildmayLine] = "#006FE6";
    _lineColorMap[Line::suffragetteLine] = "#18A95D";
    _lineColorMap[Line::weaverLine] = "#9B0058";
    _lineColorMap[Line::windrushLine] = "#DC241F";

    _lineColorMap[ Line::piccadillyLine ] = "#2E3091";
    _lineColorMap[ Line::districtLine ] = "#00A74E";
    _lineColorMap[ Line::circleLine]  = "#FBC706";
    _lineColorMap[ Line::hammersmithCityLine ] = "#DF64B9";
    _lineColorMap[ Line::metropolitanLine ] = "#B3146E";
    _lineColorMap[ Line::jubileeLine ] = "#918F8F";
    _lineColorMap[ Line::dlrLine] = "#00B1B0";

    _lineColorMap[ Line::centralLine ] = "#ED1A24";

    _lineColorMap[ Line::northernLine ] = "#221E20";
    _lineColorMap[ Line::bakerlooLine ] = "#8B4838";
    _lineColorMap[ Line::victoriaLine ] = "#0895DD";
    _lineColorMap[ Line::waterlooCityLine ] = "#7ECF7A";
    _lineColorMap["tram"] = "#99D31E";

    _lineColorMap[ Line::london_cable_car] = "#ED6579";

    _lineOffsetMap[ Line::centralLine ] = 30;
    _lineOffsetMap[ Line::victoriaLine ] = 30;
    _lineOffsetMap[ Line::circleLine ] = 15;
    _lineOffsetMap[ Line::hammersmithCityLine ] = 25;
    _lineOffsetMap[ Line::metropolitanLine ] = 60;
    _lineOffsetMap[ Line::districtLine ] = 35;
    _lineOffsetMap[ Line::piccadillyLine ] = 15;
    _lineOffsetMap[ Line::northernLine ] = 50;
    _lineOffsetMap[ Line::bakerlooLine ] = 20;
    _lineOffsetMap[ Line::dlrLine] = 35;

    _lineOffsetMap[ Line::lionessLine] = 35;
    _lineOffsetMap[ Line::mildmayLine] = 35;
    _lineOffsetMap[ Line::windrushLine] = 35;
    _lineOffsetMap[ Line::weaverLine] = 35;
    _lineOffsetMap[ Line::suffragetteLine] = 35;
    _lineOffsetMap[ Line::libertyLine] = 35;


    _lineOffsetMap[Line::jubileeLine] = 25;

}

bool TrackerGlue::doesRouteExist(QString id) const
{
    return _allRoutesList.find(id) != _allRoutesList.end();
}

bool TrackerGlue::allRoutesListsEmpty() const
{
    return _allRoutesList.isEmpty();
}

QStringList TrackerGlue::getScannedModes() const
{
    static QStringList modeList;

    if( modeList.isEmpty())
    {
        QDirIterator dirIt(":/data/Routes/inbound", QDir::Dirs|QDir::NoDotAndDotDot);

        while( dirIt.hasNext())
        {
            dirIt.next();
            modeList.push_back(dirIt.fileName());
        }
    }

    return modeList;
}

void TrackerGlue::storeAllRouteIDsInList(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonArray arr = document.array();

    _allRoutesList.clear();
    _allRoutesNamesList.clear();
    _allBusRoutesList.clear();

    for( const QJsonValue &value : std::as_const(arr))
    {
        QJsonObject obj = value.toObject();
        _allRoutesList << obj["id"].toString();
        _allRoutesNamesList << obj["name"].toString();

        if( obj["modeName"] == "bus")
            _allBusRoutesList << obj["id"].toString();
    }

#ifdef Q_OS_WIN
    qDebug() << "Available routes from TFL :" << _allRoutesList.size();
#endif
}

void TrackerGlue::loadAllRoutes()
{
    QObject::connect(_watcher, &QFutureWatcher<void>::finished, this, [this]{

        std::sort(_allLoadedRoutesList.begin(), _allLoadedRoutesList.end(), busNumberComparerId);
        emit allLinesAdded(_allLoadedRoutesList);
    });

    _loadFuture = QtConcurrent::run([this] {

        std::set<QString> localRoutesList;

        auto collectFolder = [&](QString folder, QStringList modeList) {

            for (const auto &mode : modeList)
            {
                QDirIterator dir(folder + "/inbound/" + mode, QDir::Files);

                while (dir.hasNext())
                {
                    QString next = dir.next();

                    /* Dont remove old buses
                    if (mode == QStringLiteral("bus") || mode ==QStringLiteral(""))
                    {
                        QString id = dir.fileName().remove(".txt");
                        if (_allBusRoutesList.end() == _allBusRoutesList.find(id))
                            continue;
                    }
                    */

                    if( !_allRoutesList.empty())
                    {
                        QString id = dir.fileName().remove(".txt");
                        if (_allRoutesList.end() == _allRoutesList.find(id))
                            continue;
                    }

                    localRoutesList.insert(next);
                }
            }
        };

        collectFolder(routesFolder, getScannedModes());
        collectFolder(downloadRoutesFolder, {""});

        for(auto& route: localRoutesList)
        {
            if( _watcher->isCanceled())
                return;

            QFileInfo fi(route);
            QString id = fi.completeBaseName();
            loadRoute(id);
        }
    });

    _watcher->setFuture(_loadFuture);
}

bool TrackerGlue::loadRoute(QString line)
{
    const QStringList& modeList = getScannedModes();

    bool builtInExists(false);
    bool overrideExists(false);

    std::vector<QString> folds = {"inbound", "outbound"};

    for(const auto &fold : folds)
    {
        QString timePath = QString(":/data/Times/%1/%2.txt").arg(fold, line);

        QString userPath( QString("downloads/routes/%1/%2.txt").arg(fold, line));

        overrideExists = QFile::exists(userPath);

        QString inBuilt;

        for(const auto& mode:modeList)
        {
            if(QFile::exists(inBuilt = QString(":/data/Routes/%1/%2/%3.txt").arg(fold, mode, line)))
            {
                builtInExists = true;
                break;
            }
        }

        bool useExternalPath = false;

        if( overrideExists )
        {
            if( builtInExists)
                useExternalPath = QFileInfo(userPath).lastModified() > QFileInfo(inBuilt).lastModified();
            else
                useExternalPath = true;
        }

        loadRouteFromPath(useExternalPath?userPath:inBuilt, timePath);
    }

    return builtInExists || overrideExists;
}

void TrackerGlue::loadRouteFromPath(const QString routePath, const QString timePath)
{
    QFile file(routePath);
    if( !file.open(QIODevice::ReadOnly))
        return;

    std::shared_ptr<StopPointMins> stopMins;
    if( QFile::exists(timePath))
    {
        stopMins = std::make_shared<StopPointMins>();
        if( !stopMins->Load(timePath, 2))
            stopMins.reset();
    }

    QByteArray data = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(data);

    if( doc.isNull())
    {
        qDebug() << "JSON Failed to Load : " << routePath;

    }

    std::shared_ptr<Line> line = LineBuilder().buildLine(doc);
    addRoute(line, stopMins);

    if( line->isOutbound())
    {
        _allLoadedRoutesList << line->id();
    }
}

void TrackerGlue::loadTFLModes()
{
    const QString fileName = ":/data/TFLModes/Modes.json";

    QFile file(fileName);
    if( !file.open(QIODevice::ReadOnly))
    {
        qDebug() << "TFL Modes file missing";
        return;
    }

    ui->_tflModeList.clear();

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());

    QJsonArray arr = doc.array();

    for( const QJsonValue &value : std::as_const(arr))
        ui->_tflModeList << value["modeName"].toString();
}

void TrackerGlue::stompFetch()
{
#ifdef WANT_NATIONAL_RAIL

    if( _stompNationalRail == nullptr)
    {
        _stompNationalRail.reset(new NationalRailPositionProvider{this});

        connect(_stompNationalRail.get(), &NationalRailPositionProvider::onTrainData, this,

            [this](const NationalRailPositionProvider::Train& t) {

                    ui->frameBuffer->executeOnRenderThread([t](TFLView* view) {
                        view->parseLineArrival(t);
                    });
            });
    }

    QSet<QString> lines;

    lines = _fetchNationalRailList;

    if( !_elizabethLineUsesTFL)
    {
        if( _fetchList.contains(Line::elizabethLine))
            lines.insert(Line::elizabethLine);
    }

    _stompNationalRail->fetch(lines);
#endif
}

void TrackerGlue::tflFetch()
{
     QSet<QString> tempFetchList = _fetchList;

#ifdef WANT_NATIONAL_RAIL

    if( !_elizabethLineUsesTFL)
    {
        tempFetchList.remove(Line::elizabethLine);
    }
#endif

    if( tempFetchList.empty())
        return;

    _currentlyDownloading = true;

    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QStringList itemsToRemove;

    for(auto& id : tempFetchList)
        if( !doesRouteExist(id))
            itemsToRemove << id;

    for(auto& id:itemsToRemove)
        tempFetchList.remove(id);

    QString lineList = std::accumulate(tempFetchList.begin(), tempFetchList.end(), QString(""), [](const QString &l, const QString &r) {
        return l + r + ",";
    });

    lineList = lineList.left(lineList.length()-1);

    QString urlText = _lineArrivalsURL.arg(lineList);

    QUrl url(urlText);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << "Fetch Request : " << urlText << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
        _currentlyDownloading = false;

        if( reply->error() == QNetworkReply::NoError)
        {
            QByteArray data = reply->readAll();

            ui->frameBuffer->executeOnRenderThread([data](TFLView* view) {
                view->parseLineArrival(data);
                view->setHttpLastErrMsg(QLatin1String(""));
            });
        }
        else
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
            QString strError = reply->errorString();
            QUrl urlToRemove = reply->request().url();
            strError.remove(urlToRemove.toString());
            strError.remove(urlToRemove.host());

            ui->frameBuffer->executeOnRenderThread([strError](TFLView* view) {
                view->setHttpLastErrMsg(strError);
            });
        }

        reply->deleteLater();
    });

}

void TrackerGlue::keyPressEvent(QKeyEvent *e)
{
    e->setAccepted(true);

    TFLView* view = ui->frameBuffer->getTFLView();

    if( view)
    {
        if( view->isView3D())
        {
            EulerF cam = view->getCamera();

            if( e->modifiers().testFlag(Qt::ControlModifier))
            {
                if( e->key() == Qt::Key_Space)
                {
                    cam._bank = 0.0f;
                    ui->frameBuffer->executeOnRenderThread([cam](TFLView* view) {
                        view->setCamera(cam);
                    });
                    return;
                }
            }
            else
            {
                if( e->key() == Qt::Key_Up)
                {
                    cam._pitch += 5/ view->get3DZoom();
                    ui->frameBuffer->executeOnRenderThread([cam](TFLView* view) {
                        view->setCamera(cam);
                    });
                    return;
                }

                if( e->key() == Qt::Key_Down)
                {
                    cam._pitch -= 5/ view->get3DZoom();
                    ui->frameBuffer->executeOnRenderThread([cam](TFLView* view) {
                        view->setCamera(cam);
                    });
                    return;
                }

                if( e->key() == Qt::Key_Left || e->key() == Qt::Key_Right)
                {
                    //                cam._bank -= 5/ui->radarView->get3DZoomFactor();
                    //                ui->radarView->setCamera(cam);

                    int incrAngle = (e->modifiers() & Qt::ShiftModifier) ? 10 : 1;

                    incrAngle *= e->key() == Qt::Key_Left ? -1 :1;
                    ui->frameBuffer->executeOnRenderThread([incrAngle](TFLView* view) {
                        view->setCompassValue(view->getCompassValue() + incrAngle, true);
                        view->updateViewBox();
                    });
                    return;
                }

                if( e->key() == Qt::Key_Equal)
                {
                    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                        view->set3DZoom( view->get3DZoom()+0.5f);
                    });
                    return;
                }

                if( e->key() == Qt::Key_Minus)
                {
                    ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                        view->set3DZoom( view->get3DZoom()-0.5f);
                    });
                    return;
                }
            }

            float heightJump = (e->modifiers() & Qt::ShiftModifier) ?1000:50;

            if( e->key() == Qt::Key_W)
            {
                ui->frameBuffer->executeOnRenderThread([heightJump](TFLView* view) {
                    view->increment3dHeight(heightJump);
                });
                return;
            }

            if( e->key() == Qt::Key_S )
            {
                ui->frameBuffer->executeOnRenderThread([heightJump](TFLView* view) {
                    view->increment3dHeight(-heightJump);
                });
                return;
            }
        }
        else
        {

            if( e->key() == Qt::Key_W)
            {
                ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                    view->onZoom(-0.5f);
                    view->updateViewBox(true);
                });
                return;
            }

            if( e->key() == Qt::Key_S )
            {
                ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                    view->onZoom(0.5f);
                    view->updateViewBox(true);
                });
                return;
            }
        }

        if( e->key() == Qt::Key_Q)
        {
            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->TranslatePosition(QPoint(0,-20));
            });
            return;
        }

        if( e->key() == Qt::Key_A)
        {
            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->TranslatePosition(QPoint(0,20));
            });
            return;
        }

        if( e->key() == Qt::Key_C)
        {
            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->TranslatePosition(QPoint(-20,0));
            });
            return;
        }

        if( e->key() == Qt::Key_V)
        {
            ui->frameBuffer->executeOnRenderThread([](TFLView* view) {
                view->TranslatePosition(QPoint(20,0));
            });
            return;
        }

        if( !e->modifiers().testFlag(Qt::ControlModifier))
        {
            if (e->key() == Qt::Key_Space)
            {
                ui->action_Pause->toggle();
                return;
            }
        }

        if( e->key() == Qt::Key_Z || e->key() == Qt::Key_X)
        {
            int incrAngle = (e->modifiers() & Qt::ShiftModifier) ? 10 : 1;

            incrAngle *= e->key() == Qt::Key_Z ? -1 :1;
            ui->frameBuffer->executeOnRenderThread([incrAngle](TFLView* view) {
                view->setCompassValue(view->getCompassValue() + incrAngle, true);
                view->updateViewBox();
            });
            return;
        }
    }

    e->setAccepted(false);
}

void TrackerGlue::showPurchasePage()
{
    emit showQMLPage("qmlpages/PurchasesPage.qml", QVariantList());
}

void TrackerGlue::loadOBBs()
{

#ifdef Q_OS_ANDROID
#ifdef QT_DEBUG
    QJniObject mediaDir = QJniObject::callStaticObjectMethod("android/os/Environment", "getExternalStorageDirectory", "()Ljava/io/File;");
    QJniObject mediaPath = mediaDir.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    QJniObject package = activity.callObjectMethod("getPackageName", "()Ljava/lang/String;");

    //    qDebug() << "MEDIAPATH : " << mainAbsPath;

    QDirIterator dir( mediaPath.toString()+"/Android/obb/"+package.toString(), QDir::Files);
    while( dir.hasNext())
    {
        QString next = dir.next();
        QString fname = dir.fileName();

        if( fname.endsWith(".obb"))
            QResource::registerResource(next);
    }

    QJniEnvironment env; // Don't know what this is for ?
    if (env->ExceptionCheck()) { env->ExceptionClear(); } // Or this...?
#endif
#endif

    std::vector<QString> files = {
                                  "train_routes",
                                  "tfl_meta",
                                  "raw_osm",
                                  //"osmtiles",
                                  "models",
                                  "dims",
                                  "bus",
                                  "network_rail"  };


    AssetPack assetPack("com/enthusiasticcoder/tfltracker");

    for(const QString &file : files)
    {
        QString pack;
        QString filename;
#ifdef Q_OS_ANDROID
        pack = file;
#else
        pack = "scripts";
#endif
        filename = file + ".obb";
        _obbByteArrays.push_back(assetPack.getDataFromAsset(pack, filename));
    }

    auto registerResource = [](QByteArray data, QString prefix="") {
        return QResource::registerResource(reinterpret_cast<const uchar*>(data.constData()), prefix);
    };

    for(const auto& resource: _obbByteArrays)
        registerResource(resource);
}

void TrackerGlue::loadOSMData()
{
    _osmData->importMotorway( ":/data/OSM/greater-london-latest_highway_motorway.bin");
    _osmData->importMotorway( ":/data/OSM/greater-london-latest_highway_trunk.bin");
    _osmData->importPrimary( ":/data/OSM/greater-london-latest_highway_primary.bin");
    _osmData->importSecondary( ":/data/OSM/greater-london-latest_highway_secondary.bin");

    _osmData->importTertiary(":/data/OSM/greater-london-latest_highway_tertiary.bin");
    _osmData->importTertiary(":/data/OSM/greater-london-latest_highway_unclassified.bin");
    _osmData->importTertiary(":/data/OSM/greater-london-latest_highway_service.bin");

    _osmData->importPedestrian(":/data/OSM/greater-london-latest_highway_pedestrian.bin");

    _osmData->importResidential(":/data/OSM/greater-london-latest_highway_residential.bin");

    _osmData->importFootway(":/data/OSM/greater-london-latest_highway_footway.bin");
    _osmData->importCycleWay(":/data/OSM/greater-london-latest_highway_cycleway.bin");

    _osmData->importWater(":/data/OSM/greater-london-latest_water.bin");

    _osmData->importAeroWay(":/data/OSM/greater-london-latest_aeroway_taxiway.bin");
    _osmData->importAeroRunway(":/data/OSM/greater-london-latest_aeroway_runway.bin");
}

void TrackerGlue::onDisplayInitialised()
{
    qDebug() << Q_FUNC_INFO;

    if(!_bDisplayInitialised)
    {
        initInAppStore();

        _perSecondTimer->start(1000);

        QTimer* saveTimer = new QTimer(this);
        saveTimer->setObjectName("TFTracker::saveTimer");
        connect(saveTimer, &QTimer::timeout, this, [this]
                {
                    if( _bWantToSave)
                    {
                        saveSettings();
                        _bWantToSave = false;
                    }
                });
        saveTimer->start(30000);
    }

    triggerOSM();

    loadSettings();
    updateCache();

    saveSettings();

    _bDisplayInitialised = true;
}

void setValueForActionGroup(QSettings&s, const ActionGroup* group, const QString& name )
{
    Action* action = group->checkedAction();

    if( action != nullptr)
        s.setValue(name, action->objectName());
}

void TrackerGlue::saveSettings()
{
    if( !_bSettingsLoaded)
        return;

    QSettings s;

    ui->frameBuffer->saveSettings();

    _mapTilesEntries->saveSettings();

    if(_locationInfo != nullptr)
    {
        s.setValue("GPS/UpdateInterval", _locationInfo->updateInterval());
    }

    setValueForActionGroup(s, ui->menu_Label_Bus_Verbosity, "Settings/2D/BusLabelVerbosity");
    setValueForActionGroup(s, ui->menu_Label_Train_Verbosity, "Settings/2D/TrainLabelVerbosity");

    setValueForActionGroup(s, ui->menuUnitsAltitude, "units/Altitude");
    setValueForActionGroup(s, ui->menuUnitsDistance, "units/Distance");
    setValueForActionGroup(s, ui->menuUnitsSpeed, "units/Speed");
    setValueForActionGroup(s, ui->menuUnitsVsiInterval, "units/VsiInterval");

    s.setValue("Settings/proximity/Active", ui->action_Proximity_Active->isChecked());
    s.setValue("Settings/proximity/MuteAlert", ui->action_Proximity_Mute_sound->isChecked());
    s.setValue("Settings/proximity/RingsVisible", ui->action_Show_Proximity_Rings->isChecked());
    s.setValue("Settings/Destination/PiccHeathrow", ui->action_piccadillyNormalDestination->isChecked());
    s.setValue("Settings/General/VehicleUseBehaviour", ui->action_Use_Vehicle_Behaviour->isChecked());
    s.setValue("Settings/General/ElizabethArrivalsUsesTFL", ui->action_Elizabeth_Arrivals_Use_TFL_Data->isChecked());

    const QStringList busList(_allBusRoutesList.begin(), _allBusRoutesList.end());
    s.setValue("Settings/allBusRoutesList", busList);

    const QStringList allRoutesList(_allRoutesList.begin(), _allRoutesList.end());
    s.setValue("Settings/allRoutesList", allRoutesList);

    s.setValue("InAppCheckMe", ui->action_InAppCheckMe->isChecked());

    s.setValue("view/2D/mapNight", ui->action_2D_Map_Night->isChecked());

    setValueForActionGroup(s, ui->actionGroup_skyLineGroup, "view/3D/SkyLine");

    s.setValue("view/TFL/BusStops", ui->action_TFL_BusStop_Visible->isChecked());
    s.setValue("view/TFL/BusLines", ui->action_TFL_BusLine_Visible->isChecked());
    s.setValue("view/TFL/TubeLines", ui->action_TFL_TubeLine_Visible->isChecked());

    for( auto* action : ui->menu_Circle_ID_Color_Override->actions())
        s.setValue("Settings/CircleIDOveride/" + action->objectName(), action->isChecked());

    if( !allRoutesListsEmpty())
    {
        {
            s.beginWriteArray("Settings/FetchNatBoat");
            int i =0;

            for( auto it = _NationalRailList.begin(); it != _NationalRailList.end(); ++it)
            {
                auto line = getTFLLine(*it);

                if(line == nullptr)
                    continue;

                s.setArrayIndex(i++);
                s.setValue("id", line->id());
                s.setValue("offset", line->getOffset() );
                s.setValue("color", line->getColor());
                s.setValue("visible", line->isVisible());
            }

            s.endArray();
        }

        {
            s.beginWriteArray("Settings/FetchList");
            int i =0;

            for( auto it = _fetchList.begin(); it != _fetchList.end(); ++it)
            {
                auto line = getTFLLine(*it);

                if(line == nullptr)
                    continue;

                if( line->getType() == LineType::bus)
                    continue;
                if( line->getType() == LineType::national_rail)
                    continue;

                s.setArrayIndex(i++);
                s.setValue("id", line->id());
                s.setValue("offset", line->getOffset() );
                s.setValue("color", line->getColor());
            }

            s.endArray();
        }

        {
            s.beginWriteArray("Settings/FetchBusList");
            int i =0;

            for( auto it = ui->_busList.begin(); it != ui->_busList.end(); ++it, ++i)
            {
                auto line = getTFLLine(*it);

                if(line == nullptr)
                    continue;

                s.setArrayIndex(i);
                s.setValue("id", line->id());
                s.setValue("color", line->getColor());
                s.setValue("offset", line->getOffset() );
                s.setValue("visible", line->isVisible());

                s.setValue("showStops", line->getShowStops());
                s.setValue("updateOK", line->getUpdatedOK());
                s.setValue("updateDate", line->getUpdateDate());
            }

            s.endArray();
        }
    }

    s.setValue("Settings/FetchListInitialsed", true);
}

void configCheckBox(QSettings&s, Action* action, const QString& name, bool value)
{
    action->setChecked(value);
    action->setChecked(s.value(name, value).toBool());
}

void TrackerGlue::loadSettings()
{
    QSettings s;

    const QString strBuildDateTime = s.value(BuildVersion::BuildDateTime, "").toString();
    QString strBuild;
    strBuild = QCoreApplication::applicationName();
    strBuild += " ";
    strBuild += strBuildDateTime;
    strBuild += " ";
    strBuild += QSysInfo::prettyProductName();
    _userInfo = strBuild.toLocal8Bit();

    ui->frameBuffer->executeOnRenderThread([this](TFLView* view) {
        view->setUserInfo(_userInfo);
    });

    using ConfigEntry = struct ENTRY {
        Action* a;
        QString name;
        bool value;
    };

    TFLView* v = ui->frameBuffer->getTFLView();

    std::vector<ConfigEntry> eee = {
        { ui->action_Pause,         "AppPaused", v->isPaused() },
        { ui->action_piccadillyNormalDestination, "Settings/Destination/PiccHeathrow", getPiccHeathrowDestnNormalFormat() },
        { ui->action_Use_Vehicle_Behaviour, "Settings/General/VehicleUseBehaviour", false },
        { ui->action_Elizabeth_Arrivals_Use_TFL_Data, "Settings/General/ElizabethArrivalsUsesTFL", false},
//        { ui->action_Places,        "labels/Places", v->isShowPlaces() },
//        { ui->action_3D_Show_Airport_Text, "view/3D/showAirportText", v->get3D()->isDrawAirportText() },
//        { ui->action_3D_Show_You, "view/3D/showYou", v->get3D()->getShowYou() },
//        { ui->action_2D_Show_Coastlines, "view/2D/showCoastLines", v->getDrawCoastLines() },
        { ui->action_Proximity_Active, "Settings/proximity/Active", v->getProximityActive() },
        { ui->action_Proximity_Mute_sound, "Settings/proximity/MuteAlert", false },
        { ui->action_Show_Proximity_Rings, "Settings/proximity/RingsVisible", v->getShowProximityRings()},
        { ui->action_2D_Map_Night, "view/2D/mapNight", v->isMapNight() },
        { ui->action_TFL_BusStop_Visible, "view/TFL/BusStops", v->isBusStopVisible() },
        { ui->action_TFL_BusLine_Visible, "view/TFL/BusLines", v->isBusLinesVisible() },
        { ui->action_TFL_TubeLine_Visible, "view/TFL/TubeLines", v->isTubeLinesVisible() },
//        { ui->action_Display_Short_Destination, "view/DisplayShortDestn", v->getShortDest() },
    };

    if( !_bSettingsLoaded)
        {
        for( auto& e: eee )
        {
            configCheckBox(s, e.a, e.name, e.value );
        }

        _mapTilesEntries->loadSettings();


        for( auto* action : ui->menu_Circle_ID_Color_Override->actions())
            configCheckBox(s, action, "Settings/CircleIDOveride/" + action->objectName(), false);

        ui->action_InAppCheckMe->setChecked(s.value("InAppCheckMe", false).toBool());

        const QStringList busList = s.value("Settings/allBusRoutesList").toStringList();
        _allBusRoutesList = QSet<QString>(busList.begin(), busList.end());

        if(_locationInfo != nullptr)
        {
            int updateInterval = s.value("GPS/UpdateInterval", 1000).toInt();
            qDebug() << "GPS/UpdateInterval setting to : " << updateInterval;
            _locationInfo->setUpdateInterval(updateInterval);
        }
    }
    else
    {
        for( auto& e: eee )
        {
            e.a->pulse();
        }

        for( auto* action : ui->menu_Circle_ID_Color_Override->actions())
        {
            action->pulse();
        }
    }

    QStringList triggerActions;

    triggerActions << "units/Altitude"
                   << "units/Distance"
                   << "units/Speed"
                   << "view/3D/SkyLine"
                   << "Settings/2D/BusLabelVerbosity"
                   << "Settings/2D/TrainLabelVerbosity";

    for(const QString& name : std::as_const(triggerActions))
    {
        QString triggerName = s.value(name).toString();

        if( triggerName.isEmpty())
            continue;

#ifdef Q_OS_WIN
        qDebug() << "Triggering : " << triggerName;
#endif

        Action* action = QObject::findChild<Action*>(triggerName);
        if( action != nullptr)
            action->trigger();
    }

    _bSettingsLoaded = true;
}

bool TrackerGlue::getPiccHeathrowDestnNormalFormat() const
{
    return _model->getPiccHeathrowDestnNormalFormat();
}

void TrackerGlue::checkNationalRailStns()
{
    _model->checkNationalRailStns();
}

void TrackerGlue::setGlueReady(bool fullyReady)
{
    ui->frameBuffer->executeOnRenderThread([fullyReady](TFLView* view) {
        view->setGlueReady();
        view->updateViewBox();

        if(fullyReady)
        {
            view->setReady(true);
        }
    });
}

bool TrackerGlue::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::Close)
    {
#ifdef Q_OS_WIN
        qDebug() << Q_FUNC_INFO;
#endif
        onUnintialise();
    }
    else if( e->type() == QEvent::KeyPress)
    {
        keyPressEvent(static_cast<QKeyEvent*>(e));
    }

    return QObject::eventFilter(obj, e);
}

void TrackerGlue::onUnintialise()
{
    saveSettings();

    shutDownAllTimers(this);

    if( _locationInfo != nullptr )
        _locationInfo->stopUpdates();

    if( _compass != nullptr)
        _compass->stop();
}

QString TrackerGlue::getMonthlySubscriptionCost()
{
#ifdef Q_OS_WIN
    return "$0.99";
#endif

    return _inAppStore->cost(InAppStore::AppStoreID_Monthly_Subcriber);
}

QString TrackerGlue::getLifeTimeSubscriptionCost()
{
#ifdef Q_OS_WIN
    return "$9.99";
#endif

    return _inAppStore->cost(InAppStore::AppStoreID_LifeTime_Purchase);
}

bool TrackerGlue::isMonthlySubscriptionPurchased()
{
    return _inAppStore->isPurchased(InAppStore::AppStoreID_Monthly_Subcriber);
}

bool TrackerGlue::isLifeTimeSubscriptionPurchased()
{
    return _inAppStore->isPurchased(InAppStore::AppStoreID_LifeTime_Purchase);
}

bool TrackerGlue::isMe() const
{
    if( !_inAppStore)
        return false;

    return _inAppStore->isMe();
}

MapTileEntries *TrackerGlue::getMapTileProfiles()
{
    return _mapTilesEntries;
}

void TrackerGlue::prepareAllRoutes()
{
    QString origPath = QDir::currentPath();
    QDir dir(origPath);
    dir.mkdir("downloads");
    dir.cd("downloads");
    dir.mkdir("routes");
    dir.cd("routes");
    dir.mkdir("inbound");
    dir.mkdir("outbound");
    dir.cd(origPath);

    _currentlyDownloading = true;
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QUrl url(_allAvailableBusRoutesURL);
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", _userInfo);

    QNetworkReply* reply = _manager->get(req);

#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO << ":" << QTime::currentTime().toString();
#endif

    connect(reply, &QNetworkReply::finished, this, [reply, this]
    {
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO << ":" << QTime::currentTime().toString();
#endif
        reply->deleteLater();
        _currentlyDownloading = false;

        QByteArray data = reply->readAll();

        if( reply->error() == QNetworkReply::NoError)
        {
            storeAllRouteIDsInList(data);

            _allLoadedRoutesRefreshed = true;

            ui->frameBuffer->executeOnRenderThread([data](TFLView* view) {
                view->setHttpLastErrMsg(QStringLiteral(""));
            });
        }
        else
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
            QString strError = reply->errorString();
            QUrl urlToRemove = reply->request().url();
            strError.remove(urlToRemove.toString());
            strError.remove(urlToRemove.host());

            ui->frameBuffer->executeOnRenderThread([strError](TFLView* view) {
                view->setHttpLastErrMsg(strError);
            });
        }
    });
}

void TrackerGlue::setUpFetchList()
{
    QSettings s;
    bool fetchListInit = s.value("Settings/FetchListInitialsed", false).toBool();

    auto saveLineEntry = [&s](TFLLine* line) {
        line->setColor( QColor(s.value("color").toString()));
        line->setOffSet(s.value("offset").toInt());
    };

    if( fetchListInit )
    {
        {
            _NationalRailList.clear();
            int count = s.beginReadArray("Settings/FetchNatBoat");

            for( int i=0; i < count; ++i)
            {
                s.setArrayIndex(i);
                auto line = getTFLLine(s.value("id").toString());

                if( line == nullptr)
                    continue;

                line->setVisible(s.value("visible").toBool());

                if( !line->isReadOnly())
                    saveLineEntry(line);

                _NationalRailList << line->id();
            }

            s.endArray();
        }

        {
            _fetchList.clear();
            int count = s.beginReadArray("Settings/FetchList");

            for( int i=0; i < count; ++i)
            {
                s.setArrayIndex(i);
                auto line = getTFLLine(s.value("id").toString());

                if( line == nullptr)
                    continue;

                if( line->isNationalRail())
                    continue;

                if( !line->isReadOnly())
                    saveLineEntry(line);

                line->setVisible(_fetchList.size() < getFetchListLimit());

                if( line->isVisible())
                    _fetchList << line->id();
            }

            s.endArray();
        }

        {
            ui->_busList.clear();

            int count = s.beginReadArray("Settings/FetchBusList");
            for( int i=0; i < count; ++i)
            {
                s.setArrayIndex(i);

                QString id = s.value("id").toString();

                auto line = getTFLLine(id);

                if( line == nullptr)
                    continue;

                saveLineEntry(line);

                line->setUpdatedOK(s.value("updateOK").toBool());
                line->setUpdateDate(s.value("updateDate").toDateTime());
                line->setShowStops(s.value("showStops").toBool());

                if( _fetchList.size() < getFetchListLimit())
                    line->setVisible(s.value("visible").toBool());
                else
                    line->setVisible(false);

                ui->_busList << line->id();

                if( line->isVisible() )
                    _fetchList << line->id();
            }

            s.endArray();
        }
    }
    else
    {
        _fetchList << "central";

        ui->_busList << "94";

        for(auto& bus:ui->_busList)
            _fetchList << bus;

        for(auto it = _fetchList.begin(); it != _fetchList.end(); ++it)
        {
            auto line = getTFLLine(*it);

            if( line == nullptr)
                continue;

            line->setVisible(true);

            if( *it == "94")
                line->setColor("blue");
            else if( *it == "23")
                line->setColor("green");
        }
    }

    updateFetchFilterList();
}

void TrackerGlue::downloadBusRoute(QString lineId)
{
    QNetworkReply* reply = downloadBusRoute(lineId, "inbound");

    connect(reply, &QNetworkReply::finished, this, [reply, lineId, this]
    {
        if( reply->error() == QNetworkReply::NoError)
        {
            QNetworkReply* reply = downloadBusRoute(lineId, "outbound");

            connect(reply, &QNetworkReply::finished, this, [reply, lineId, this]
            {
                if( reply->error() == QNetworkReply::NoError)
                {
                    loadRoute(lineId);

                    ui->frameBuffer->executeOnRenderThread([lineId](TFLView* view) {
                        view->removeVehicles(lineId);
                    });
                    emit tflRouteDownloaded(lineId);
                }
            });
        }
    });
}

QNetworkReply* TrackerGlue::downloadBusRoute(QString lineId, QString direction)
{
    QUrlQuery query;
    query.addQueryItem("app_id", appID);
    query.addQueryItem("app_key", key);

    QUrl url(_routeSequence.arg(lineId).arg(direction));
    url.setQuery(query);

    QNetworkRequest req(url);
    req.setRawHeader("User-Agent" , _userInfo);

    QNetworkReply* reply = _manager->get(req);

    connect(reply, &QNetworkReply::finished, this, [reply, lineId, this]
    {
        reply->deleteLater();

        if( reply->error() != QNetworkReply::NoError)
        {
#ifdef Q_OS_WIN
            qDebug() << reply->errorString();
#endif
            emit tflRouteFailedToDownload(lineId);
        }
        else
        {
            QByteArray result = reply->readAll();
            saveRoute(result);
       }
    });

    return reply;
}



void TrackerGlue::saveRoute(const QByteArray &json)
{
    QJsonDocument document = QJsonDocument::fromJson(json);
    QJsonObject rootObj = document.object();

    QJsonDocument finalDocument;

    QJsonObject topObject;
    topObject["lineId"] = rootObj["lineId"];
    topObject["lineName"] = rootObj["lineName"];

    QString currentLineId = rootObj["lineId"].toString();
    bool bInbound = rootObj["direction"].toString().startsWith("inbound");

    QString mode = rootObj["mode"].toString();
    if( mode ==Line::mode_tube
            || mode == Line::mode_tram
            || mode == Line::mode_nationalRail
            || mode == Line::mode_dlr
            || mode == Line::mode_overground
            || mode == Line::mode_elizabeth)
        return;

    topObject["mode"] = mode;

    topObject["direction"] = rootObj["direction"];

    QJsonArray inBranchArray = rootObj["stopPointSequences"].toArray();

    QJsonArray outBranchArray;

    for(const QJsonValue &value : inBranchArray)
    {
        QJsonObject obj;

        QJsonArray inStopPointArray = value["stopPoint"].toArray();
        QJsonArray outStopPointArray;
        for(const QJsonValue &value : inStopPointArray)
        {
            QJsonObject obj;

            obj["parentId"] = value["parentId"];
            obj["stationId"] = value["stationId"];
            obj["icsId"] = value["icsId"];
            obj["topMostParentId"] = value["topMostParentId"];
            obj["stopLetter"] = value["stopLetter"];
            obj["towards"] = value["towards"];
            obj["zone"] = value["zone"];

            obj["id"] = value["id"];
            obj["name"] = value["name"];
            obj["lat"] = value["lat"];
            obj["lon"] = value["lon"];
            outStopPointArray.append(obj);
        }

        obj["stopPoint"] = outStopPointArray;

        outBranchArray.append(obj);
    }

    topObject["stopPointSequences"] = outBranchArray;

    finalDocument.setObject(topObject);

    QString outfilename(QString("downloads/routes/%2/%1.txt").arg(currentLineId, bInbound?"inbound":"outbound"));

    QFile::remove(outfilename);

    QFile file(outfilename);
    if( !file.open(QIODevice::WriteOnly))
    {
        emit tflRouteFailedToDownload(currentLineId);
        return;
    }

    QTextStream textStream(&file);
    textStream << finalDocument.toJson(QJsonDocument::Compact);
    file.close();
}
