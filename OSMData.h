#ifndef OSMDATA_H
#define OSMDATA_H

#include <vector>
#include <QString>
#include <jibbs/gps/GPSLocation.h>
#include <jibbs/gps/GPSTileContainer.h>

struct WAYPOINTS
{
    std::vector<std::pair<qsizetype,QString>> tags;
    std::vector<GPSLocation> gpsPts;
    std::vector<int> bearings;
};

using WAYPOINTS_TILES = GPSTileContainer<WAYPOINTS*>;

#define OSMType(name, wayPts) \
    void import##name(const QString& filename) { import(filename, wayPts);}\
    const WAYPOINTS_TILES& get##name() const { return wayPts;}

class OSMData
{
public:
    OSMData();

    OSMType(Motorway, _osmMotorway)
    OSMType(Primary, _osmPrimary)
    OSMType(Secondary, _osmSecondary)
    OSMType(Tertiary, _osmTertiary)
    OSMType(Residential, _osmResidential)
    OSMType(Footway, _osmFootway)
    OSMType(Water, _osmWater)
    OSMType(AeroWay, _osmAeroway)
    OSMType(AeroRunway, _osmAeroRunway)
    OSMType(CycleWay, _osmCycleway)
    OSMType(Pedestrian, _osmPedestrian);

    void setViewBoundary(std::pair<GPSLocation, GPSLocation> b);

protected:
    void import(const QString &filename, WAYPOINTS_TILES &tiles, bool bAllowPoints=false);
    std::pair<GPSLocation, GPSLocation> adjustRange(const GPSLocation& middle, int distance) const;

private:
    WAYPOINTS_TILES _osmMotorway;
    WAYPOINTS_TILES _osmPrimary;
    WAYPOINTS_TILES _osmSecondary;
    WAYPOINTS_TILES _osmTertiary;
    WAYPOINTS_TILES _osmResidential;
    WAYPOINTS_TILES _osmFootway;
    WAYPOINTS_TILES _osmWater;
    WAYPOINTS_TILES _osmAeroway;
    WAYPOINTS_TILES _osmAeroRunway;
    WAYPOINTS_TILES _osmCycleway;
    WAYPOINTS_TILES _osmPedestrian;
    QuarternionF _topLeft;
    QuarternionF _bottomRight;
};

#endif // OSMDATA_H
