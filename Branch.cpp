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

QVector<Branch::TempTrackPt> Branch::buildSmoothTracks(bool bReverse)
{
    QVector<TempTrackPt> tmpTrack;
    std::shared_ptr<Line> line = parent;

    if (_ids.size() < 2) return tmpTrack;

    auto& qHdgTable = QHdgTable::get();
    QVector<std::shared_ptr<StopPoint>> stops;

    for (int i = 0; i < _ids.size(); ++i)
        stops << line->getStopPoint(_ids[i]);

    if (bReverse)
        std::reverse(stops.begin(), stops.end());

    const float distStep = 20.0f;
    const float arcRadius = 80.0f;

    for (int i = 0; i < stops.size() - 1; ++i)
    {
        auto a = stops[i]->position;
        auto b = stops[i + 1]->position;
        float totalDist = a.distanceTo(b);
        float bearing = a.bearingTo(b);

        int nSteps = std::max(2, static_cast<int>(totalDist / distStep));
        for (int s = 0; s < nSteps; ++s)
        {
            float f = static_cast<float>(s) / nSteps;
            GPSLocation pt = a.interpolateTo(b, f);

            TempTrackPt ttp;
            ttp.pos = pt;
            ttp.hdg = bearing;
            ttp.stopPoint = (s == 0 ? stops[i] : nullptr);
            tmpTrack << ttp;
        }
    }

    // Final point
    TempTrackPt last;
    last.pos = stops.back()->position;
    last.hdg = tmpTrack.back().pos.bearingTo(last.pos);
    last.stopPoint = stops.back();
    tmpTrack << last;

    // Apply lateral offset
    int offset = (_offset > 0) ? _offset : line->getOffset();
    Vector3F vOffset(0, 0, -offset);
    for (auto& pt : tmpTrack)
        pt.pos += QVRotate(qHdgTable.Hdg(pt.hdg - 90), vOffset);

    return tmpTrack;
}
