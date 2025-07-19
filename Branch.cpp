#include <jibbs/math/qhdgtable.h>
#include <jibbs/math/MathSupport.h>

#include "Branch.h"
#include <memory>
#include "Line.h"
#include "TurnDirection.h"

int Branch::getId() const
{
    return _id;
}

bool Branch::isActive() const
{
    return !_smoothPoints.empty();
}

void Branch::build(bool bActive)
{
    clearSmoothTrackPoints();

    if( bActive)
        generateSmoothTrackPoints();
}

const TrackPoints &Branch::getSmoothPoints()
{
    return _smoothPoints;
}

const std::vector<ColorStnPoint> &Branch::getSmoothStnPoints()
{
    return _stnPoints;
}

std::vector<GPSLocation> &Branch::getStnPoints()
{
    return _stnPositions;
}

void Branch::clearSmoothTrackPoints()
{
    _smoothPoints.clear();
    _stnPoints.clear();
}

void Branch::generateSmoothTrackPoints()
{
    if( !_smoothPoints.empty())
        return;

    const bool bReverse = !parent->isOutbound();

    if( _ids.size() < 2)
        return;

    auto& qHdgTable = QHdgTable::get();

    auto tmpTrk = buildSmoothTracks(bReverse);

    int stopIndex = -1;
    int prevStopIndex = stopIndex;
    for( const auto& trkPt : std::as_const(tmpTrk))
    {
        std::shared_ptr<TrackPoint> trackPoint = std::make_shared<TrackPoint>();
        trackPoint->position = trkPt.pos;
        trackPoint->hdg = trkPt.hdg;
        trackPoint->stopPoint = trkPt.stopPoint;
        trackPoint->prevStopOffset = ++stopIndex;

        if( trkPt.stopPoint && (!trkPt.stopPoint->isPassPoint || !trkPt.stopPoint->id.startsWith("{")))
        {
            BranchConnect bc;
            bc.branch = this;
            bc.idx = static_cast<int>(_smoothPoints.size());

            auto& aB = trkPt.stopPoint->attachedBranches;
            auto it = std::find(aB.begin(), aB.end(), bc);

            if( it == aB.end())
                trkPt.stopPoint->attachedBranches.push_back(bc);

            stopIndex = 0;

            const float fHdg = trackPoint->hdg - 90;//+ (bReverse? 90: -90);
            const GPSLocation stnGPS = trkPt.stopPoint->position + QVRotate(qHdgTable.Hdg(fHdg),
                                                                 Vector3F(0,0,-parent->getOffset()));

            ColorStnPoint coloredPt;
            coloredPt.hdg = trkPt.hdg;
            coloredPt.pos = stnGPS;
            _stnPoints.push_back(coloredPt);
        }

        if( prevStopIndex ==-1 && stopIndex == 0 && !_smoothPoints.empty())
        {
            prevStopIndex = stopIndex;
            _smoothPoints[0]->prevStopOffset = trackPoint->prevStopOffset;
        }

        _smoothPoints.push_back(trackPoint);
    }
}

QVector<Branch::TempTrackPt> Branch::buildSmoothTracks(bool /*bReverse*/)
{
    QVector<TempTrackPt> tmpTrack;
    std::shared_ptr<Line> line = parent;
    auto& qHdgTable = QHdgTable::get();

    if (_ids.size() < 2) return tmpTrack;

    QVector<std::shared_ptr<StopPoint>> stops;
    for (const auto& id : _ids)
    {
        auto stop = line->getStopPoint(id);
        if (stop) stops << stop;
    }

    QVector<GPSLocation> pts;
    for (const auto& s : stops)
        pts << s->position;

    const int samplesPerSegmentBus = 6;
    const int samplesPerSegmentTrain = 3;
    const int samplesPerSegment = line->isTrain() ? samplesPerSegmentTrain : samplesPerSegmentBus;

    auto interpolate = [](const GPSLocation& p0, const GPSLocation& p1, const GPSLocation& p2, const GPSLocation& p3, float t) {
        float t2 = t * t;
        float t3 = t2 * t;
        return p1 * 2.0f +
               (p2 - p0) * t +
               (p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
               (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3;
    };

    for (int i = 0; i < pts.size() - 1; ++i)
    {
        const auto& p0 = pts[std::max(0, i - 1)];
        const auto& p1 = pts[i];
        const auto& p2 = pts[i + 1];
        const auto& p3 = pts[std::min((int)pts.size() - 1, i + 2)];

        for (int j = 0; j < samplesPerSegment; ++j)
        {
            float t = float(j) / samplesPerSegment;
            GPSLocation pos = interpolate(p0, p1, p2, p3, t) * 0.5f;

            TempTrackPt pt;
            pt.pos = pos;
            pt.stopPoint = (j == 0) ? stops[i] : nullptr;
            tmpTrack << pt;
        }
    }

    // Final stop point
    TempTrackPt last;
    last.pos = pts.last();
    last.stopPoint = stops.last();
    tmpTrack << last;

    // Single pass: heading + offset
    int offset = (_offset > 0) ? _offset : line->getOffset();
    Vector3F vOffset;

    for (int i = 0; i < tmpTrack.size(); ++i)
    {
        if (i < tmpTrack.size() - 1)
            tmpTrack[i].hdg = tmpTrack[i].pos.bearingTo(tmpTrack[i + 1].pos);
        else if (i > 0)
            tmpTrack[i].hdg = tmpTrack[i - 1].hdg;

        vOffset = Vector3F(0, 0, -offset);
        tmpTrack[i].pos += QVRotate(qHdgTable.Hdg(tmpTrack[i].hdg - 90), vOffset);
    }

    return tmpTrack;
}
