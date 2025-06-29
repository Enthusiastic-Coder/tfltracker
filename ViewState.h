#ifndef VIEWSTATE_H
#define VIEWSTATE_H

#include <jibbs/gps/GPSLocation.h>
#include <QSize>
#include <QTransform>

class WorldModel;

struct ViewState {
    GPSLocation gpsOrigin;    // Origin location
    float pixelsPerMile;      // Scaling factor
    QSize middleOfScreen;         // Middle of screen
    QTransform invCompassTransform; // Compass transformation matrix
    bool mapNight;
    QSize size;
    std::shared_ptr<const WorldModel> world;
    float compassValue;
    QRect boundaryFromRange;
    std::pair<GPSLocation, GPSLocation> boundaryView;
    float devicePixelRatio;
};

#endif // VIEWSTATE_H
