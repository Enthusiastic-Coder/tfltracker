#ifndef BRANCH_H
#define BRANCH_H

#include <jibbs/gps/GPSLocation.h>

#include <QString>
#include <vector>
#include <QVector>

#include "TrackPoint.h"

class Line;

struct ColorStnPoint
{
    int hdg = 0;
    GPSLocation pos;
};

class Branch
{
public:
    std::shared_ptr<Line> parent;
    friend class LineBuilder;

    struct TempTrackPt
    {
        std::shared_ptr<StopPoint> stopPoint;
        int hdg = -1000;
        GPSLocation pos;
    };

    int getId() const;
    bool isActive() const;
    void build(bool bActive);

    const TrackPoints& getSmoothPoints();
    const std::vector<ColorStnPoint> &getSmoothStnPoints();
    std::vector<GPSLocation>& getStnPoints();

protected:
    void clearSmoothTrackPoints();
    void generateSmoothTrackPoints();
    QVector<TempTrackPt> buildSmoothTracks(bool bReverse);

private:
    int _id = -1;
    int _offset = 0;
    TrackPoints _smoothPoints;
    std::vector<ColorStnPoint> _stnPoints;
    std::vector<QString> _ids;
    std::vector<GPSLocation> _stnPositions;
};

#endif // BRANCH_H
