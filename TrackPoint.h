#pragma once

#include <jibbs/gps/GPSLocation.h>
#include <memory.h>
#include <vector>

class StopPoint;

struct TrackPoint
{
    std::shared_ptr<StopPoint> stopPoint;
    int hdg = -1;
    int prevStopOffset = 0;
    GPSLocation position;
};

using TrackPoints = std::vector<std::shared_ptr<TrackPoint>>;

