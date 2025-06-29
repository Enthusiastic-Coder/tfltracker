#ifndef AIRCRAFTLOGOPOINTS_H
#define AIRCRAFTLOGOPOINTS_H

#include <vector>
#include <map>
#include <jibbs/vector/vector3.h>
#include <QPointF>

using AcPoints = std::map<int,std::vector<std::vector<QPointF>>>;
using AcFixedPoints = std::map<int,std::vector<QPointF>>;

class RadarSymbols
{
public:
    const std::vector<QPointF>& GetAircraftBlip(int decimalScale, int hdg);
    const std::vector<QPointF>& GetHelicopterBlip(int decimalScale, int hdg);
    const std::vector<QPointF>& GetGliderBlip(int decimalScale, int hdg);
    const std::vector<QPointF>& GetBalloonBlip(int decimalScale, int hdg);
    const std::vector<QPointF>& GetTriangleArrowBlip(int decimalScale, int hdg);
    const std::vector<QPointF>& GetWideBarBlip(int decimalScale, int hdg);

    const std::vector<QPointF>& GetPerpendicularTrail(int decimalScale, int hdg);
    const std::vector<QPointF>& GetBlipFuture(int decimalScale, int hdg);
    const std::vector<QPointF>& GetSimpleBlip(int decimalScale, int hdg);

    const std::vector<QPointF>& GetLondonUndergroupSymbol(int decimalScale);

private:
    AcPoints _acPoints;
    AcPoints _helicopterPoints;
    AcPoints _gliderPoints;
    AcPoints _balloonPoints;
    AcPoints _perpendicularTrailPoints;
    AcPoints _wideBarPoints;
    AcPoints _futureBlipPoints;
    AcPoints _simpleBlipPoints;
    AcFixedPoints _londonUndergroundPoints;
};

#endif // AIRCRAFTLOGOPOINTS_H
