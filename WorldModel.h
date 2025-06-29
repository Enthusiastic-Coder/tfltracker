#ifndef MODEL_H
#define MODEL_H

#include <jibbs/gps/GPSLocation.h>

#include <QVector>
#include <QString>
#include <QColor>
#include <QMap>
#include <QJsonDocument>
#include <QtXml>

#include <map>
#include <deque>

#include <QPainter>
#include <memory>

#include "Line.h"
#include "Vehicle.h"
#include "TFLModel.h"

#include <jibbs/gps/GPSTileContainer.h>
#include "NationalRailCRC.h"
#include "NationalRailPositionProvider.h"

struct OldStnTrackPts;
struct Station;
class SubLine;
class StopPointMins;

class WorldModel
{
public:
    WorldModel();

    int vehicleCount() const;

    void initNationalRail();
    void addRoute(std::shared_ptr<Line> line, std::shared_ptr<StopPointMins> timeData);

    QMap<QString, std::shared_ptr<Vehicle> > getTrains() const;

    QMap<QString, std::shared_ptr<Line> > getLines() const;
    std::shared_ptr<Line> getLine(const QString &direction, const QString& id) const;

    std::shared_ptr<StopPoint> findStationId(const QString& id) const;
    std::shared_ptr<StopPoint> findNationalRailID(const QString& id) const;
    QString findCRCFromStationName(const QString& name) const;
    const QSet<QString> getLineNamesForStationId(const QString& id) const;
    const QSet<QString> getLineIDsForStationId(const QString& id) const;

    void parseLineArrival(const QDomDocument& doc, QString lineId);
    void parseLineArrival(const QJsonDocument &doc);
    void parseLineArrival(const NationalRailPositionProvider::Train &t);
    void removeAllVehicles();
    void removeVehicles(QString line);
    void removeOldVehicles();
    void update();

    void clearSelectedVehicles();
    size_t getSelectedVehiclesCount() const;
    const std::shared_ptr<Vehicle> getSelectedVehicle(size_t idx) const;
    const std::shared_ptr<Vehicle> getVehicle(const QString &key) const;

    void setSelectedVehicle(const QString &id);
    void selectVehicleInArea(const GPSLocation &topLeft, const GPSLocation &bottomRight);
    std::vector<QString> getVehiclesInArea(const GPSLocation &topLeft, const GPSLocation &bottomRight);

    void setPiccHeathrowDestnNormalFormat(bool normal);
    bool getPiccHeathrowDestnNormalFormat() const;

    void setCircleColorOverride(QString id, bool on);
    void setUseVehicleBehaviour(bool b);

    void checkNationalRailStns();

    void setGPSOrigin(const GPSLocation &loc);

    std::deque<GPSLocation> _GPSHistory;

protected:
    void placeFakeVehicles(QMap<QString, std::shared_ptr<Vehicle> > &trains);
    void placeVehiclesOnLines(QMap<QString, std::shared_ptr<Vehicle> > &trains);
    void adjustOverlappingVehicles();
    QString makeId(const QString& id, const QString& direction) const;
    QString keyForVehicle(QString modeName, QString id, QString naptanId, QString lineId, QString vehicleId, QString direction, QString destinationName) const;

private:
    QMap<QString, std::shared_ptr<Vehicle> > parseVehicles(QJsonArray items);

private:
    bool _bHeathrowNormalDestn=false;
    std::map<QString, bool> _CircleLineOverrideMap;

    QMap<QString,std::shared_ptr<Line>> _lines;

    QMap<QString, std::shared_ptr<Vehicle>> _vehicles;
    QHash<QString, QString> _previousStopPoint;
    bool _useVehicleBehaviour = false;

    std::vector<QString> _selectedVehicles;
    GPSLocation _gpsOrigin;

    std::map<QString,std::shared_ptr<StopPoint>> _stopIdsStopPoint;
    std::map<QString,std::shared_ptr<StopPoint>> _nationalRailStopPoint;
    std::map<QString,QSet<QString>> _stopPointLineNames;
    std::map<QString,QSet<QString>> _stopPointLineIDs;

    NationalRailCRC _nationalRailFile;
};



#endif // MODEL_H
