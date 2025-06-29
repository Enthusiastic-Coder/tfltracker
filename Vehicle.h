#pragma once

#include "helpers.h"
#include <QString>
#include <QTime>
#include <vector>

#include "BranchConnect.h"

class Line;
class Branch;
struct TrackPoint;
class StopPoint;
struct PossibleAttactment;

enum DataSource {
    DataSource_Unknown,
    DataSource_TFL,
    DataSource_NetworkRail,
};

struct Vehicle
{
    enum CurrentBehaviour
    {
        Unknown,
        At,
        Approaching,
        Between,
        Departed
    };

    DataSource _dataSource = DataSource_Unknown;

    QTime lastUpdated;
    bool _fast = false;
    bool _hailAndRide = false;
    bool _auxilaryTube = false;
    QString _key;
    QString _vehicleId;
    QString _stationName;
    QString _naptanId;
    int _timeToStation = 0;
    int _timeInBetweenStation = 0;
    QString _currentLocation;
    QString _nextLocation;
    QString _previousLocation;
    QString _towards;
    QString _originName;
    QString _destinationName;
    QString _modeName;
    QString _lineId;
    QString _direction;
    QString _platform;
    int _platformOffSet = 0;

    QString _displayStationName;
    QString _displayTowards;

    bool _useVehicleBehaviour = false;
    bool _wantToBeCircle = false;

    std::shared_ptr<Line> _line;
    BranchConnect _attachment;
    float _trackOffSet = 0.0f;
    const TrackPoint* getTrackPoint() const;
    GPSLocation getPos() const;
    CurrentBehaviour _behaviour = Unknown;

    bool isFast() const;
    bool isBus() const;
    bool isDLR() const;
    bool isTube() const;
    bool isOverground() const;
    bool isElizabeth() const;
    bool isRiverBoat() const;
    bool isNationalRail() const;

    QString currentBehaviour() const;

    bool isHammersmithyCity() const;
    bool isCircle() const;
    bool isPiccadilly() const;
    bool isCentral() const;
    bool isDistrict() const;
    bool isJubilee() const;
    bool isNorthern() const;
    bool isBakerloo() const;
    bool isMetropolitan() const;
    bool isLondonOverground() const;
    bool isTram() const;
    bool isVictoria() const;

    QString toString() const;

    void qqDebug(QString title) const;

    void updatePosAndDir();
    void updatePreviousStopCache(QHash<QString, QString> &previousStopPoint);
    bool placeOnTrack();

    void updateHeathrowTowards(bool b);
    void update();

    bool nextStopTheSame(const std::shared_ptr<Vehicle> &vehicle) const;

    QString getDataSource() const;

private:
    std::shared_ptr<const StopPoint> _nextStopPoint;
    std::shared_ptr<const StopPoint> _prevStopPoint;

private:
    void cleanUpLocation( QString& location);
    void updateDirectionIfBlank();
    void updateDirectionOverride();
    void updateNextStopIfNeeded();
    void updateDirectionIfNeeded();
    void updateDisplayTowards();
    void updateHammtoCircleIfNeeded();
    void updateBehaviourPosition();
    void updateIfUpminsterOnHighStKenBranch();
    std::vector<Branch*> buildBranchExcludeList(std::shared_ptr<const StopPoint> stopPoint);
    std::vector<PossibleAttactment> getPossibleAttachments( const std::vector<Branch*>& excludeList);
    float calcTrackPos(const BranchConnect& attach);
};
