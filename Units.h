#ifndef UNITS_H
#define UNITS_H

#include <functional>
#include <map>
#include <QString>
#include <QObject>

class Units
{
public:
    Units();

    enum class Distance
    {
        NauticalMiles,
        StatueMiles,
        Meters,
        Kilometers
    };

    enum class Altitude
    {
        Feet,
        Meters,
        NauticalMiles,
        StatueMiles,
        Kilometers
    };

    enum class Speed
    {
        Knots,
        Kilometers,
        Miles,
        Meters
    };

    enum class PerInterval
    {
        Minute,
        Second
    };

    void setDistance(Distance dist);
    void setAltitude(Altitude alt);
    void setSpeed(Speed spd);
    void setVsiPerInteraval(PerInterval i);

    float getDistance(float nm) const;
    float getAltitude(float feet) const;
    float getSpeed(float kts) const;
    float getInterval(float i) const;

    float getInvAltitude(float value) const;
    float getInvDistance(float value) const;
    float getInvSpeed(float value) const;

    QString getDistName() const;
    QString getSpeedName() const;
    QString getAltitudeName() const;
    QString getVsiIntervalName() const;

    Altitude getAltitudeType() const;
    PerInterval getVsiIntervalType() const;

    static float FtToMeters(float ft);
    static float FtToNM(float ft);
    static float FtToSM(float ft);
    static float FtToKM(float ft);

    static float MetersToNm(float m);
    static float MetersToFeet(float m);
    static float MetersToKts(float m);

    static float KtsToKM(float kts);
    static float KtsToMPH(float kts);
    static float KtsToMeters(float kts);

    static float NmToMeters(float nm);
    static float NmToKM(float nm);
    static float NmToSM(float nm);

private:
    Distance _distanceType = Distance::NauticalMiles;
    Altitude _altitudeType = Altitude::Feet;
    Speed _speedType = Speed::Knots;
    PerInterval _VSIintervalType = PerInterval::Minute;

    QString _distName;
    QString _altName;
    QString _speedName;
    QString _VSIName;

    std::function<float(float)> _spdDelegate;
    std::function<float(float)> _altDelegate;
    std::function<float(float)> _distDelegate;
    std::function<float(float)> _VSIDelegate;

    std::map<Distance,std::pair<QString,std::function<float(float)>>> _distMap;
    std::map<Speed,std::pair<QString,std::function<float(float)>>> _speedMap;
    std::map<Altitude,std::pair<QString, std::function<float(float)>>> _altMap;
    std::map<PerInterval,std::pair<QString, std::function<float(float)>>> _vsiMap;

};


Q_DECLARE_METATYPE(Units::Distance)
Q_DECLARE_METATYPE(Units::Speed)
Q_DECLARE_METATYPE(Units::Altitude)
Q_DECLARE_METATYPE(Units::PerInterval)

#endif // UNITS_H
