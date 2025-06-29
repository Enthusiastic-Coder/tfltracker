#include <jibbs/math/qhdgtable.h>
#include <jibbs/math/Quarternion.h>

#include <QDebug>
#include "RadarSymbols.h"

namespace {

std::vector<QPointF> GetDefaultAircraftPoints()
{
    std::vector<QPointF> pts0;
    pts0.push_back({0, -4});
    pts0.push_back({0, 8});

    pts0.push_back({0, 2});
    pts0.push_back({-5, 5});

    pts0.push_back({0, 2});
    pts0.push_back({5, 5});

    pts0.push_back({0, 8});
    pts0.push_back({-2, 10});

    pts0.push_back({0, 8});
    pts0.push_back({2, 10});
    return pts0;
}

std::vector<QPointF> GetDefaultHelicopterPoints()
{
    // -5, +5 -> x
    // -10, +10 ->y

    std::vector<QPointF> pts0;
    pts0.push_back({-2, -8});
    pts0.push_back({2, -8});

    pts0.push_back({2, -8});
    pts0.push_back({2, 2});

    pts0.push_back({2, 2});
    pts0.push_back({-2, 2});

    pts0.push_back({-2, 2});
    pts0.push_back({-2, -8});

  //////////////////////
    pts0.push_back({-8, -8 });
    pts0.push_back({8, 0});

    pts0.push_back({8, -8 });
    pts0.push_back({-8, 0});

    /////////////////////////

    pts0.push_back({0, 0 });
    pts0.push_back({0, 8});

    pts0.push_back({0, 8 });
    pts0.push_back({-2, 10});

    pts0.push_back({0, 8 });
    pts0.push_back({2, 10});

    return pts0;
}

std::vector<QPointF> GetDefaultGliderPoints()
{
    // -5, +5 -> x
    // -10, +10 ->y

    std::vector<QPointF> pts0;
    pts0.push_back({0, -6});
    pts0.push_back({0, 6});

    pts0.push_back({-11, -2});
    pts0.push_back({11, -2});

    pts0.push_back({2, 6});
    pts0.push_back({-2, 6});

    return pts0;
}

std::vector<QPointF> GetDefaultBalloonPoints()
{
    // -5, +5 -> x
    // -10, +10 ->y

    const float sz = 4.0f;
    std::vector<QPointF> pts0;
    pts0.push_back({-sz, -sz});
    pts0.push_back({sz, -sz});

    pts0.push_back({sz, -sz});
    pts0.push_back({sz, sz});

    pts0.push_back({sz, sz});
    pts0.push_back({-sz, sz});

    pts0.push_back({-sz, sz});
    pts0.push_back({-sz, -sz});

    return pts0;
}

std::vector<QPointF>GetDefaultTrianglePoints()
{
    const float sz = 10.0f;
    std::vector<QPointF> pts0;

    pts0.push_back({-sz, sz});
    pts0.push_back({sz, sz});

    pts0.push_back({sz, sz});
    pts0.push_back({0, -sz});

    pts0.push_back({0, -sz});
    pts0.push_back({-sz, sz});

    return pts0;
}

std::vector<QPointF> GetDefaultWideBarPoints()
{
    // -5, +5 -> x
    // -10, +10 ->y

    std::vector<QPointF> pts0;
    pts0.push_back({-8, -4});
    pts0.push_back({8, -4});

    pts0.push_back({8, -4});
    pts0.push_back({8, 4});

    pts0.push_back({8, 4});
    pts0.push_back({-8, 4});

    pts0.push_back({-8, 4});
    pts0.push_back({-8, -4});

    return pts0;
}

std::vector<QPointF> GetDefaultPerpendicularPoints()
{
    // -5, +5 -> x
    // -10, +10 ->y

    std::vector<QPointF> pts0;
    pts0.push_back({-10, 0});
    pts0.push_back({10, 0});

    return pts0;
}

std::vector<QPointF> GetBlipFuturePoints()
{
    std::vector<QPointF> pts0;
    pts0.push_back({0, 0});
    pts0.push_back({0, -1});

    return pts0;
}

std::vector<QPointF> GetSimpleBlipPoints()
{
    std::vector<QPointF> pts0;
    pts0.push_back({0, 0});
    pts0.push_back({-5, 5});

    pts0.push_back({0, 0});
    pts0.push_back({5, 5});

    pts0.push_back({0, 0});
    pts0.push_back({0, 10});

    return pts0;
}

std::vector<QPointF> GetDefaultLondonUndergroundPoints()
{
    std::vector<QPointF> pts0;
    pts0.push_back({-3, -3});
    pts0.push_back({3, -3});

    pts0.push_back({3, -3});
    pts0.push_back({3, 3});

    pts0.push_back({3, 3});
    pts0.push_back({-3, 3});

    pts0.push_back({-3, 3});
    pts0.push_back({-3, -3});

    pts0.push_back({-5, 0});
    pts0.push_back({5, 0});
    return pts0;
}


void BuildPoints(std::vector<QPointF> (*defaultPoints)(), AcPoints& bluePrint, int id, float fScale)
{
#ifdef Q_OS_WIN
    qDebug() << "BuildPoints:" << id << "," << fScale;
#endif
    auto acpt0 = (*defaultPoints)();

    std::transform(acpt0.begin(), acpt0.end(), acpt0.begin(), [fScale](const QPointF& pt) {
       return pt * fScale;
    });

    std::vector<std::vector<QPointF>>& hdgPts = bluePrint[id];
    hdgPts.clear();

    for( int hdg = 0; hdg <=360; ++hdg)
    {
        auto acpts = acpt0;
        const QuarternionF& qHdg = QHDG(hdg);

        for(size_t i=0; i < acpts.size(); i++)
        {
            Vector3F pt(acpts[i].x(), 0, acpts[i].y());
            pt = QVRotate(qHdg, pt);
            acpts[i].setX(pt.x);
            acpts[i].setY(pt.z);
        }

        hdgPts.push_back(acpts);
    }
}


const std::vector<QPointF>& GetLogo(std::vector<QPointF> (*defaultPoints)(), AcPoints &bluePrint, int decimalScale, int hdg)
{
    auto it = bluePrint.find(decimalScale);

    if( it == bluePrint.end())
    {
        BuildPoints(defaultPoints, bluePrint, decimalScale, decimalScale/10.0f);
        it = bluePrint.find(decimalScale);

        if( it == bluePrint.end())
        {
            static const auto defPoints = GetDefaultAircraftPoints();
            return defPoints;
        }
    }

    return (it->second)[MathSupport<int>::normAng(hdg)];
}


}

const std::vector<QPointF>& RadarSymbols::GetAircraftBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultAircraftPoints, _acPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetHelicopterBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultHelicopterPoints, _helicopterPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetGliderBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultGliderPoints, _gliderPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetBalloonBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultBalloonPoints, _balloonPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetTriangleArrowBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultTrianglePoints, _balloonPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetWideBarBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultWideBarPoints, _wideBarPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetPerpendicularTrail(int decimalScale, int hdg)
{
    return GetLogo(&GetDefaultPerpendicularPoints, _perpendicularTrailPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetBlipFuture(int decimalScale, int hdg)
{
    return GetLogo(&GetBlipFuturePoints, _futureBlipPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetSimpleBlip(int decimalScale, int hdg)
{
    return GetLogo(&GetSimpleBlipPoints, _simpleBlipPoints, decimalScale, hdg);
}

const std::vector<QPointF> &RadarSymbols::GetLondonUndergroupSymbol(int decimalScale)
{
    auto it = _londonUndergroundPoints.find(decimalScale);

    if( it == _londonUndergroundPoints.end())
    {
        _londonUndergroundPoints[decimalScale] = GetDefaultLondonUndergroundPoints();
        auto& points = _londonUndergroundPoints[decimalScale];
        for(auto& point : points)
        {
            point *= decimalScale/10.0f;
        }
    }

    return _londonUndergroundPoints[decimalScale];
}





