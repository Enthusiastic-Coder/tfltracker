#include "Units.h"

auto spdToKnots = [](float v) { return v; };
auto spdToKilometers = [] (float v){ return Units::KtsToKM(v); };
auto spdToMiles = [] (float v){ return Units::KtsToMPH(v); };
auto spdToMeters = [] (float v){ return Units::KtsToMeters(v); };

auto distToNM = [](float v) { return v; };
auto distToSM = [](float v) { return Units::NmToSM(v); };
auto distToM = [](float v) { return Units::NmToMeters(v); };
auto distToKM = [](float v) { return Units::NmToKM(v); };

auto altToFT = [](float v) { return v; };
auto altToMeters = [](float v) { return Units::FtToMeters(v); };
auto altToNM = [](float v) { return Units::FtToNM(v);};
auto altToSM = [](float v) { return Units::FtToSM(v);};
auto altToKM = [](float v) { return Units::FtToKM(v);};

auto vsiToMin = [](float v) { return v; };
auto vsiToSec = [](float v) { return v/60.0f;};

Units::Units()
{
    qRegisterMetaType<Units::Distance>();
    qRegisterMetaType<Units::Speed>();
    qRegisterMetaType<Units::Altitude>();
    qRegisterMetaType<Units::PerInterval>();

    _distMap[Distance::Kilometers]  = {"Km", distToKM};
    _distMap[Distance::Meters]      = {"Meters", distToM };
    _distMap[Distance::NauticalMiles] = {"Nm", distToNM} ;
    _distMap[Distance::StatueMiles] = {"M", distToSM};

    _speedMap[Speed::Kilometers] = {"Km/h", spdToKilometers};
    _speedMap[Speed::Knots] = {"Kts", spdToKnots };
    _speedMap[Speed::Meters] = {"m/s", spdToMeters};
    _speedMap[Speed::Miles] = {"Mph", spdToMiles };

    _altMap[Altitude::Feet] = {"ft", altToFT};
    _altMap[Altitude::Meters] = {"m", altToMeters};
    _altMap[Altitude::NauticalMiles] = {"Nm", altToNM};
    _altMap[Altitude::StatueMiles] = {"M", altToSM};
    _altMap[Altitude::Kilometers] = {"Km", altToKM};

    _vsiMap[PerInterval::Minute] = {"/min", vsiToMin};
    _vsiMap[PerInterval::Second] = {"/s", vsiToSec};

    setSpeed(Speed::Knots);
    setAltitude(Altitude::Feet);
    setDistance(Distance::NauticalMiles);
    setVsiPerInteraval(PerInterval::Minute);
}

void Units::setDistance(Units::Distance dist)
{
    _distanceType = dist;
    _distName = _distMap[dist].first;
    _distDelegate = _distMap[dist].second;
}

void Units::setAltitude(Units::Altitude alt)
{
    _altitudeType = alt;
    _altName = _altMap[alt].first;
    _altDelegate = _altMap[alt].second;
}

void Units::setSpeed(Units::Speed spd)
{
    _speedType = spd;
    _speedName = _speedMap[spd].first;
    _spdDelegate = _speedMap[spd].second;
}

void Units::setVsiPerInteraval(Units::PerInterval i)
{
    _VSIintervalType = i;
    _VSIName = _vsiMap[i].first;
    _VSIDelegate = _vsiMap[i].second;
}

float Units::getDistance(float nm) const
{
    return _distDelegate(nm);
}

float Units::getAltitude(float feet) const
{
    return _altDelegate(feet);
}

float Units::getSpeed(float kts) const
{
    return _spdDelegate(kts);
}

float Units::getInterval(float i) const
{
    return _VSIDelegate(i);
}

float Units::getInvAltitude(float value) const
{
    return value/_altDelegate(1.0f);
}

float Units::getInvDistance(float value) const
{
    return value/_distDelegate(1.0f);
}

float Units::getInvSpeed(float value) const
{
    return value/_spdDelegate(1.0f);
}

QString Units::getDistName() const
{
    return _distName;
}

QString Units::getSpeedName() const
{
    return _speedName;
}

QString Units::getAltitudeName() const
{
    return _altName;
}

QString Units::getVsiIntervalName() const
{
    return _VSIName;
}

Units::Altitude Units::getAltitudeType() const
{
    return _altitudeType;
}

Units::PerInterval Units::getVsiIntervalType() const
{
    return _VSIintervalType;
}

float Units::NmToMeters(float nm)
{
    return nm * 1.15f * 1.609334f * 1000;
}

float Units::NmToKM(float nm)
{
    return nm* 1.15f * 1.609334f;
}

float Units::NmToSM(float nm)
{
    return nm * 1.15f;
}

float Units::MetersToNm(float m)
{
    return m * 3.2808f / 5280 / 1.15f;
}


float Units::FtToMeters(float ft)
{
    return ft / 3.2808f;
}

float Units::FtToNM(float ft)
{
    return ft/5280.0f/1.15f;
}

float Units::FtToSM(float ft)
{
    return ft/5280.0f;
}

float Units::FtToKM(float ft)
{
    return ft/5280.0f*1.609334f;
}

float Units::MetersToFeet(float m)
{
    return m * 3.2808f;
}

float Units::MetersToKts(float m)
{
    return m * 3600 / 1000.0f / 1.609334f / 1.15f;
}

float Units::KtsToMeters(float kts)
{
    return kts* 1.15f* 1609.334f / 3600;
}

float Units::KtsToKM(float kts)
{
    return kts * 1.15f * 1.609934f;
}

float Units::KtsToMPH(float kts)
{
    return kts * 1.15f;
}
