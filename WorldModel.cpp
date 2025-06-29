#include <jibbs/utilities/stdafx.h>
#include <jibbs/math/MathSupport.h>

#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>
#include <QVariant>
#include <QMetaEnum>
#include <QJsonValue>

#include "WorldModel.h"
#include "Line.h"
#include "Branch.h"
#include "StopPointMins.h"

WorldModel::WorldModel()
{

}

int WorldModel::vehicleCount() const
{
    return _vehicles.size();
}

void WorldModel::initNationalRail()
{
    _nationalRailFile.Load(":/data/NetworkRail/nr_station_codes.csv", 2);
}

void WorldModel::addRoute(std::shared_ptr<Line> line, std::shared_ptr<StopPointMins> timeData)
{
    QString id = makeId( line->id(), line->direction());

    _lines[id] = line;

    const auto& stopPoints = line->getStopPoints();
    for( const auto& stopPoint : stopPoints)
    {
        if( timeData && !stopPoint.second->isPassPoint)
            stopPoint.second->timeTo = timeData->getMins(stopPoint.second->displayName);

        bool isNationalRail(false);
        if( line->isNationalRail() || line->isOverground() || line->isElizabethRail())
        {
            QString CRC = _nationalRailFile.getCRCFromStnName(stopPoint.second->displayName);
            if( (isNationalRail = !CRC.isEmpty()))
                _nationalRailStopPoint[CRC] = stopPoint.second;
        }

        if( !line->isNationalRail())
            _stopIdsStopPoint[stopPoint.second->id] = stopPoint.second;

        _stopPointLineNames[stopPoint.second->id].insert(line->name());
        _stopPointLineIDs[stopPoint.second->id].insert(line->id());
    }
}

QString WorldModel::keyForVehicle(QString modeName, QString id, QString naptanId, QString lineId, QString vehicleId, QString direction, QString destinationName)  const
{
    QString key;

    if(modeName == Line::mode_busLine || modeName == Line::mode_elizabeth || modeName == Line::mode_overground)
    {
        key = vehicleId;
    }
    else if(modeName == Line::dlrLine)
    {
        key = QString("dlr:%1:%2:%3").arg(naptanId, direction, destinationName);

        auto it = _stopIdsStopPoint.find(naptanId);

        if( it != _stopIdsStopPoint.end())
        {
            vehicleId = "["+it->second->name+"]";
            cleanStationName(vehicleId);
        }
    }
    else if(lineId == Line::circleLine || lineId == Line::districtLine || lineId == Line::hammersmithCityLine)
    {
         key = QString("%1:%2").arg(vehicleId, "cir-ham-dist");
    }
    else
    {
        if(vehicleId == QStringLiteral("000"))
            key = QString("%1:%2:%3").arg(id, vehicleId, lineId);
        else
            key = QString("%1:%2").arg(vehicleId, lineId);
    }

    return key;
}

QMap<QString, std::shared_ptr<Vehicle>> WorldModel::parseVehicles( QJsonArray items)
{
    QMap<QString, std::shared_ptr<Vehicle>> vehicles;

    for(const auto i : items)
    {
        QJsonObject obj = i.toObject();
        QString modeName = obj[QStringLiteral("modeName")].toString();

        int timeToStation = obj[QStringLiteral("timeToStation")].toInt();

        int timeLimit = 500;

        if(modeName == Line::mode_dlr)
            timeLimit = 200;

        else if( modeName == Line::mode_tube)
            timeLimit = 200;

        else if( modeName == Line::mode_overground)
            timeLimit = 300;

        else if( modeName == Line::mode_nationalRail)
            timeLimit = 3000;

        else if( modeName == Line::mode_elizabeth)
            timeLimit = 3000;

        else if( modeName == Line::mode_busLine)
            timeLimit = 500;

        if( timeToStation > timeLimit)
            continue;

        QString id = obj[QStringLiteral("id")].toString();
        QString vehicleId = obj[QStringLiteral("vehicleId")].toString();
        QString stationName = obj[QStringLiteral("stationName")].toString();
        QString destinationName = obj[QStringLiteral("destinationName")].toString();
        QString direction = obj[QStringLiteral("direction")].toString();

        QString lineId = obj[QStringLiteral("lineId")].toString();
        QString towards = obj[QStringLiteral("towards")].toString();

        if(vehicleId == QStringLiteral("000"))
            continue;

        bool isHailAndRide = false;

        if( modeName == Line::mode_busLine)
        {
            isHailAndRide = stationName.startsWith(QStringLiteral("Hail & Ride Starts"))
                    || stationName.startsWith(QStringLiteral("Hail & Ride End"))
                    || stationName.startsWith(QStringLiteral("Hail & Ride Section"));


            if( !isHailAndRide)
            {
                QString hailRide = QStringLiteral("Hail & Ride ");
                if( (stationName.indexOf(hailRide) != -1) )
                    stationName.remove(hailRide);
            }
        }

        if( lineId == Line::circleLine || lineId == Line::hammersmithCityLine)
        {
            if( direction == Line::inbound)
            {
                auto it = _CircleLineOverrideMap.find(vehicleId);

                if( it != _CircleLineOverrideMap.end() && it->second )
                    lineId = Line::circleLine;
            }
        }

        QString naptanId = obj[QStringLiteral("naptanId")].toString();

        QString key = keyForVehicle(modeName, id, naptanId, lineId, vehicleId, direction, destinationName );

        std::shared_ptr<Vehicle>& v = vehicles[key];

        if( !v)
        {
            v.reset(new Vehicle);
        }

        if( v->_timeToStation == 0 || v->_timeToStation > timeToStation)
        {
            v->_dataSource = DataSource_TFL;
            v->_timeToStation = timeToStation;
            v->_key = key;
            v->_hailAndRide = isHailAndRide;
            v->_vehicleId = vehicleId;
            v->_stationName = stationName;
            v->_naptanId = naptanId;
            v->_displayStationName = obj[QStringLiteral("stationName")].toString();
            v->_currentLocation = obj[QStringLiteral("currentLocation")].toString();
            v->_towards = towards;
            v->_destinationName = destinationName;
            v->_modeName = modeName;
            v->_direction = direction;
            v->_platform = obj[QStringLiteral("platformName")].toString();
            v->_lineId = lineId;
            v->_useVehicleBehaviour = _useVehicleBehaviour;
        }
    }

    return vehicles;
}

void WorldModel::parseLineArrival(const QDomDocument &doc, QString lineId)
{
    QMap<QString, std::shared_ptr<Vehicle>> trains;

    QDomNode n = doc.firstChild().firstChild();

    while( !n.isNull())
    {
        if( n.isElement())
        {
            QDomElement e = n.toElement();

            QString stnName = e.attribute("N");
            stnName.remove(QChar('.'));

            QDomNode platform = n.firstChild();

            while( !platform.isNull())
            {
                if( platform.isElement())
                {
                    QDomElement e = platform.toElement();
                    QString platformName = e.attribute("N");

                    QDomNode trainElements = e.firstChild();

                    while( !trainElements.isNull())
                    {
                        if( trainElements.isElement())
                        {
                            QDomElement e = trainElements.toElement();

                            QString vehicleId = e.attribute("S");
                            QString destinationName = e.attribute("DE");

                            QString key = keyForVehicle(Line::mode_tube, "", "", lineId, vehicleId, "", destinationName );

                            auto it = _vehicles.find(key);

                            if( it == _vehicles.end() || it.value()->_auxilaryTube)
                            {
                                std::shared_ptr<Vehicle>& v = trains[key];

                                if( !v)
                                {
                                    v.reset(new Vehicle);
                                }

                                QString timeTo = e.attribute("C");
                                timeTo = timeTo.left(timeTo.indexOf(":"));
                                int timeToStation = timeTo.toInt()*60;

                                QString location = e.attribute("L");
                                if( location == QStringLiteral("At Platform"))
                                    location = destinationName;

                                if( v->_timeToStation == 0 || v->_timeToStation > timeToStation)
                                {
                                    v->_auxilaryTube = true;
                                    v->_currentLocation = location;
                                    v->_vehicleId = vehicleId;
                                    v->_platform = platformName;
                                    v->_destinationName = destinationName;
                                    v->_timeToStation = timeToStation;
                                    v->_key = key;
                                    v->_stationName = stnName;
                                    v->_displayStationName = stnName;
                                    v->_towards = destinationName;
                                    v->_modeName = Line::mode_tube;
                                    v->_direction = "";
                                    v->_lineId = lineId;
                                    v->_useVehicleBehaviour = _useVehicleBehaviour;
                                }
                            }
                        }

                        trainElements = trainElements.nextSibling();
                    }
                }

                platform = platform.nextSibling();
            }
        }

        n = n.nextSibling();
    }

#ifdef Q_OS_WIN
    qDebug () << "TUBE AUXILARY COUNT : " << trains.size();
    for(const auto& item : std::as_const(trains))
        qDebug() << item->toString();
    qDebug () << "TUBE AUXILARY ============================";
#endif
    placeVehiclesOnLines(trains);
}

void WorldModel::parseLineArrival(const QJsonDocument &doc)
{
    QMap<QString, std::shared_ptr<Vehicle>> trains;

    trains = parseVehicles(doc.array());

    placeFakeVehicles(trains);

    placeVehiclesOnLines(trains);

#ifdef Q_OS_WIN2
    qDebug() << "PREV-STOP-POINTS:";
    for (auto it = _previousStopPoint.begin();it != _previousStopPoint.end();++it)
    {
        qDebug() << it.key() << ":" << it.value();
    }
#endif
}

void WorldModel::parseLineArrival(const NationalRailPositionProvider::Train &t)
{
    QMap<QString, std::shared_ptr<Vehicle>> trains;

    std::shared_ptr<Vehicle>& v = trains[t.vehicleId];

    v.reset(new Vehicle);

    v->_dataSource = DataSource_NetworkRail;

    v->_timeToStation = t.timeTo * 60;
    v->_key = t.vehicleId;
    v->_vehicleId = t.vehicleId;
    v->_stationName = t.nextStn;

    if( t.eventType == QStringLiteral("PASS"))
    {
        v->_currentLocation = t.locStn;
    }
    else
    {
        v->_currentLocation = t.reportingStn;
    }

    v->_displayStationName = t.nextStn;

    v->_towards = t.nextStn;

    v->_originName = t.originName;
    v->_destinationName = t.destinationName;

    if( t.lineId == Line::elizabethLine)
    {
        v->_modeName = Line::mode_elizabeth;
    }
    else
    {
        v->_modeName = Line::mode_nationalRail;
    }

    v->_direction = t.direction;
    v->_platform = t.platform;
    v->_lineId = t.lineId;
    v->_useVehicleBehaviour = false;

    if( t.eventType == "ARRIVAL")
    {
        v->_behaviour = Vehicle::Approaching;
    }
    if( t.eventType == "DEPARTURE")
    {
        v->_behaviour = Vehicle::Departed;
    }

    placeVehiclesOnLines(trains);
}

void WorldModel::removeAllVehicles()
{
    _vehicles.clear();
}

void WorldModel::removeVehicles(QString line)
{
    QStringList keysToRemove;

    for( auto& vehicle : _vehicles)
    {
        if( vehicle->_lineId.compare(line, Qt::CaseInsensitive) == 0)
            keysToRemove << vehicle->_key;
    }

    for( auto& key : keysToRemove)
    {
#ifdef Q_OS_WIN
//        qDebug() << "Removed Vehicle : " << key;
#endif
        _vehicles.remove(key);
    }
}

void WorldModel::placeVehiclesOnLines(QMap<QString, std::shared_ptr<Vehicle>> &trains)
{
    QTime curTime = QTime::currentTime();

    const auto keys = trains.keys();
    for( auto& key : keys)
    {
        std::shared_ptr<Vehicle>& vehicle = trains[key];

        if( vehicle->_lineId.isEmpty())
            continue;

        if( !vehicle->isBus())
        {
            cleanStationName(vehicle->_stationName);
            cleanStationName(vehicle->_originName);
            cleanStationName(vehicle->_destinationName);
        }

        vehicle->updatePosAndDir();

        // Check if direction is blank
        if( vehicle->_direction.isEmpty())
        {
//            vehicle.qqDebug("NO DIRECTION:");
            std::shared_ptr<Line> line = getLine( Line::outbound, vehicle->_lineId);

            if( line == nullptr)
            {
#ifdef Q_OS_WIN
                qDebug() << "************MISSED********************" << vehicle->_lineId;
#endif
                continue;
            }

            const int offSet = line->getStopPointDiff( vehicle->_nextLocation.isEmpty()?vehicle->_stationName:vehicle->_nextLocation,
                                            vehicle->_destinationName);

            //vehicle._nextLocation.isEmpty()?vehicle._destinationName

            if( offSet > 0)
                vehicle->_direction = Line::outbound;
            else
                vehicle->_direction = Line::inbound;

#ifdef Q_OS_WIN
            vehicle->qqDebug("Dir Selected");
#endif
        }

        auto it = _vehicles.find(key);

        if( it != _vehicles.end())
            vehicle->_wantToBeCircle |= it.value()->_wantToBeCircle;

        if( vehicle->_wantToBeCircle)
            vehicle->_lineId = Line::circleLine;

        vehicle->_line = getLine( vehicle->_direction, vehicle->_lineId);

        if( it != _vehicles.end())
        {
            vehicle->_trackOffSet = it.value()->_trackOffSet;
            vehicle->_attachment = it.value()->_attachment;
        }

        if( !vehicle->isBus())
            vehicle->updatePreviousStopCache(_previousStopPoint);

        if( !vehicle->placeOnTrack() && it == _vehicles.end())
            continue;

        if( it != _vehicles.end())
            if( it.value()->nextStopTheSame(vehicle))
                if( vehicle->_behaviour == it.value()->_behaviour)
                {
                    vehicle->_timeToStation = it.value()->_timeToStation;
                    vehicle->_timeInBetweenStation = it.value()->_timeInBetweenStation;
                }

        std::shared_ptr<Vehicle>& liveTrain = it == _vehicles.end() ? _vehicles[key] : *it;
        liveTrain = vehicle;
        liveTrain->lastUpdated = curTime;
    }
}

void WorldModel::adjustOverlappingVehicles()
{
    std::multimap<std::pair<Branch*,int>,std::shared_ptr<Vehicle>> overlapTrainMap;

    const auto& keys = _vehicles.keys();
    for( auto& key : keys)
    {
        std::shared_ptr<Vehicle>& vehicle = _vehicles[key];

        if( vehicle->isBus())
            continue;

        overlapTrainMap.insert({
                                   {vehicle->_attachment.branch, vehicle->_trackOffSet/4}, vehicle}
                               );
    }

    for(auto it = overlapTrainMap.begin(); it != overlapTrainMap.end();)
    {
        auto currIt = it;
        int offSet = 0;
        while( it != overlapTrainMap.end() && currIt->first == it->first)
        {
            it->second->_platformOffSet = offSet++;
            ++it;
        }
    }
}

QString WorldModel::makeId(const QString &id, const QString &direction) const
{
    return QString("%1.%2").arg(id, direction);
}

QMap<QString, std::shared_ptr<Vehicle>> WorldModel::getTrains() const
{
    return _vehicles;
}

QMap<QString, std::shared_ptr<Line> > WorldModel::getLines() const
{
    return _lines;
}

std::shared_ptr<Line> WorldModel::getLine( const QString& direction, const QString &id) const
{
    auto it = _lines.find(makeId( id, direction));
    if( it == _lines.end())
        return nullptr;

    return it.value();
}

std::shared_ptr<StopPoint> WorldModel::findStationId(const QString &id) const
{
    auto it = _stopIdsStopPoint.find(id);
    if( it == _stopIdsStopPoint.end())
        return nullptr;

    return it->second;
}

std::shared_ptr<StopPoint> WorldModel::findNationalRailID(const QString &id) const
{
    auto it = _nationalRailStopPoint.find(id);
    if( it == _nationalRailStopPoint.end())
        return nullptr;

    return it->second;
}

QString WorldModel::findCRCFromStationName(const QString &name) const
{
    return _nationalRailFile.getCRCFromStnName(name);
}

const QSet<QString> WorldModel::getLineNamesForStationId(const QString &id) const
{
    auto it = _stopPointLineNames.find(id);
    if( it == _stopPointLineNames.end())
        return QSet<QString>();

    return it->second;
}

const QSet<QString> WorldModel::getLineIDsForStationId(const QString &id) const
{
    auto it = _stopPointLineIDs.find(id);
    if( it == _stopPointLineIDs.end())
        return QSet<QString>();

    return it->second;
}

void WorldModel::removeOldVehicles()
{
    QVector<QString> keysToRemove;

    QTime currentTime = QTime::currentTime();

    for(auto it = _vehicles.begin(); it != _vehicles.end(); ++it)
    {
        auto& train = it.value();

        const int timeOut = train->_dataSource == DataSource_NetworkRail ? 240 : 120;

        if( train->lastUpdated.secsTo(currentTime) > timeOut )
        {
            keysToRemove << it.key();
        }
    }

    for( auto& key : keysToRemove)
    {
#ifdef Q_OS_WIN
//        qDebug() << "Removed Vehicle : " << key;
#endif
        _vehicles.remove(key);
    }
}

void WorldModel::update()
{
    std::map<Branch*,std::vector<std::shared_ptr<Vehicle>>> duplicateVehicleMap;

    for(auto& vehicle : _vehicles)
    {
        vehicle->update();
        vehicle->updateHeathrowTowards(_bHeathrowNormalDestn);

        if( vehicle->isDLR())
            duplicateVehicleMap[vehicle->_attachment.branch].push_back(vehicle);
    }

    for(auto& item : duplicateVehicleMap)
    {
        std::vector<std::shared_ptr<Vehicle>>& sortedVehicles = item.second;

        std::sort(sortedVehicles.begin(), sortedVehicles.end(), [](const std::shared_ptr<Vehicle>& left, const std::shared_ptr<Vehicle>& right) {
            return left->_trackOffSet > right->_trackOffSet;
        });

        QVector<QString> keysToRemove;
        QString lastDestination;
        GPSLocation lastLocation;

        for(std::shared_ptr<Vehicle>& vehicle : sortedVehicles)
        {
            if( lastDestination.compare(vehicle->_destinationName, Qt::CaseInsensitive) == 0 )
            {
                int dist = 1100;

                if( vehicle->isDLR())
                    dist = 750;

                if( vehicle->getPos().distanceTo(lastLocation) <dist)
                {
                    keysToRemove << vehicle->_key;


#ifdef Q_OS_WIN
                    qDebug() << "Removed Duplicate " << vehicle->_lineId <<  " : " << vehicle->_destinationName << ":" << vehicle->_direction << ":" << vehicle->_displayStationName;
#endif
                }
            }

            lastDestination = vehicle->_destinationName;
            lastLocation = vehicle->getPos();
        }

        for( auto& key : keysToRemove)
            _vehicles.remove(key);
    }

    adjustOverlappingVehicles();

    _GPSHistory.push_back(_gpsOrigin);
    if( _GPSHistory.size() > 5*60)
        _GPSHistory.pop_front();
}

void WorldModel::setGPSOrigin(const GPSLocation &loc)
{
    _gpsOrigin = loc;
}

void WorldModel::clearSelectedVehicles()
{
    _selectedVehicles.clear();
}

size_t WorldModel::getSelectedVehiclesCount() const
{
    return _selectedVehicles.size();
}

const std::shared_ptr<Vehicle> WorldModel::getSelectedVehicle(size_t idx) const
{
    if( idx >= _selectedVehicles.size())
        return nullptr;

    const auto& it = _vehicles.find(_selectedVehicles[idx]);
    if( it == _vehicles.end())
        return nullptr;

    return *it;
}

const std::shared_ptr<Vehicle> WorldModel::getVehicle(const QString& key) const
{
    auto it = _vehicles.find(key);

    if( it == _vehicles.end())
        return nullptr;

    return *it;
}

void WorldModel::setSelectedVehicle(const QString &id)
{
    clearSelectedVehicles();
    _selectedVehicles.push_back(id);
}

void WorldModel::selectVehicleInArea(const GPSLocation& topLeft, const GPSLocation& bottomRight)
{
    _selectedVehicles = getVehiclesInArea(topLeft, bottomRight);
}

std::vector<QString> WorldModel::getVehiclesInArea(const GPSLocation& topLeft, const GPSLocation& bottomRight)
{
    std::vector<QString> selected;

    std::vector<std::shared_ptr<const Vehicle>> blips;

    for (const auto& pBlip : std::as_const(_vehicles))
    {
        if( pBlip->getPos().ptInBounds(topLeft, bottomRight))
            blips.push_back( pBlip);
    }

    GPSLocation middle = (topLeft + bottomRight) * 0.5;

    std::sort(blips.begin(), blips.end(), [&middle](const std::shared_ptr<const Vehicle>& l, const std::shared_ptr<const Vehicle>& r )
    {
        return l->getPos().distanceTo(middle) < r->getPos().distanceTo(middle);
    }
    );

    selected.resize(blips.size());

    std::transform(blips.begin(), blips.end(), selected.begin(), [](const std::shared_ptr<const Vehicle>& item) {
        return item->_key;
    });

    return selected;
}

void WorldModel::placeFakeVehicles(QMap<QString, std::shared_ptr<Vehicle>> &trains)
{
//vehicleID:072 [district]
//Stn:Edgware Road (Circle Line) Underground Station
//Plat:Platform 2
//NaptanId:940GZZLUERC
//OrigCurr:[At Platform]
//Curr:[Platform]
//Toward:Wimbledon
//Dir:inbound
//TimeTo:16 [0.10m][0mins]
//Dest:Wimbledon Underground Station
//ETA:16:05:49

//    Vehicle& t001 = trains["custom001"];
//    t001._modeName = "tube";
//    t001._key = "custom001";
//    t001._lineId = "northern";
//    t001._platform = "Southbound - Platform 6";
//    t001._currentLocation = "Approaching Euston Platform 2";
//    t001._vehicleId = "999";
//    t001._towards = "Kennington via CX";
//    t001._destinationName = "Kennington Underground Station";
//    t001._stationName = "Euston Underground Station";
//    t001._direction = "inbound";
//    t001._timeToStation = 35;
//    t001._useVehicleBehaviour = true;


//    Vehicle& t001 = trains ["custom001"];
//    t001._modeName = Line::mode_tube;
//    t001._naptanId = "940GZZLUHR4";
//    t001._key = "custom001";
//    t001._lineId = Line::piccadillyLine;
//    t001._currentLocation = "Between Hatton Cross and Heathrow Terminal 4";
//    t001._vehicleId = "999";
//    t001._towards = "Heathrow via T4 Loop";
//    t001._direction = "";
//    t001._destinationName = "Heathrow Terminal 4 Underground Station";
//    t001._stationName = "Heathrow Terminal 4 Underground Station";
//    t001._timeToStation = 100;

//    Vehicle& t001 = trains["custom001"];
//    t001._modeName = "tube";
//    t001._key = "custom001";
//    t001._lineId = "metropolitan";
//    t001._currentLocation = "Between Finchely Road and St";
//    t001._vehicleId = "999";
//    t001._towards = "";
//    t001._direction = "inbound";
//    t001._destinationName = "Amersham";
//    t001._stationName = "Finchley Road";
//    t001._timeToStation = 300;

//    Vehicle& t002 = trains["custom002"];
//    t002._modeName = "tube";
//    t002._key = "custom002";
//    t002._lineId = "metropolitan";
//    t002._currentLocation = "Between Finchely Road and St";
//    t002._vehicleId = "998";
//    t002._towards = "";
//    t002._direction = "outbound";
//    t002._destinationName = "Aldgate";
//    t002._stationName = "Finchley Road";
//    t002._timeToStation = 300;

//vehicleID:411 [metropolitan]
//Stn:Moorgate Underground Station
//Plat:Eastbound - Platform 1
//NaptanId:940GZZLUMGT
//OrigCurr:[Left Barbican]
//Curr:[Barbican]
//Toward:Aldgate
//Dir:inbound
//TimeTo:85 [0.53m][1mins]
//Dest:Aldgate Underground Station
//ETA:16:20:53


//    Vehicle& t001 = trains["custom001"];
//    t001._modeName = "tube";
//    t001._key = "custom001";
//    t001._lineId = "metropolitan";
//    t001._currentLocation = "Left Barbican";
//    t001._vehicleId = "999";
//    t001._towards = "Aldgate";
//    t001._direction = "inbound";
//    t001._destinationName = "Aldgate Underground Station";
//    t001._stationName = "Moorgate Underground Station";
//    t001._timeToStation = 300;

}

void WorldModel::setPiccHeathrowDestnNormalFormat(bool normal)
{
    _bHeathrowNormalDestn = normal;
}

bool WorldModel::getPiccHeathrowDestnNormalFormat() const
{
    return _bHeathrowNormalDestn;
}

void WorldModel::setCircleColorOverride(QString id, bool on)
{
    _CircleLineOverrideMap[id] = on;
#ifdef Q_OS_WIN
    qDebug() << Q_FUNC_INFO << ":" << id << "-" << on;
#endif
}

void WorldModel::setUseVehicleBehaviour(bool b)
{
    _useVehicleBehaviour = b;
}

void WorldModel::checkNationalRailStns()
{
    qDebug() << Q_FUNC_INFO;

    const auto& lines = getLines();
    for(const auto&line : lines)
    {
        if( !line->isNationalRail())
            continue;

        const auto& stopPoints = line->getStopPoints();

        for(const auto& stopPoint : stopPoints)
        {
            if( stopPoint.second->isPassPoint)
                continue;

            QString CRC = _nationalRailFile.getCRCFromStnName(stopPoint.second->displayName);
            if( CRC.isEmpty())
                qDebug() << "MISSING CRC : " << line->id() << " : " << stopPoint.second->displayName;

        }
    }
}
