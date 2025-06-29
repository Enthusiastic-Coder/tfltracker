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
    std::shared_ptr<Line> line = parent;

    QVector<TempTrackPt> tmpTrack;

    auto addTmpPt = [&tmpTrack](const GPSLocation& walker, std::shared_ptr<StopPoint> currentStopPoint=nullptr) {
        TempTrackPt pt;
        pt.pos = walker;
        pt.stopPoint = currentStopPoint;
        tmpTrack << pt;
    };

    float hdgWalker = 0;
    bool hdgWalkInit(false);
    int currentStopPointIndex = bReverse ? static_cast<int>(_ids.size())-1 : 0;
    auto& qHdgTable = QHdgTable::get();

    std::shared_ptr<StopPoint> currentStopPoint = line->getStopPoint(_ids[currentStopPointIndex]);
    GPSLocation walker = currentStopPoint->position;

    do
    {
        if( bReverse)
            currentStopPointIndex--;
        else
            currentStopPointIndex++;

        std::shared_ptr<StopPoint> nextStopPoint = line->getStopPoint(_ids[currentStopPointIndex]);

        const float distInterval = 50;
        const float turnPerInterval = distInterval/3.33f;

        float brg = walker.bearingTo(nextStopPoint->position);

        if( !hdgWalkInit || currentStopPoint->instantTurn)
        {
            hdgWalkInit = true;
            hdgWalker = brg;
        }

        addTmpPt(walker, currentStopPoint);
        currentStopPoint = nextStopPoint;

        while( walker.distanceTo(nextStopPoint->position) > distInterval*2)
        {
            if( !line->isTrain() || walker.distanceTo(nextStopPoint->position) < distInterval*2)
            {
                hdgWalker = brg;
            }
            else
            {
                TurnDirection::Dir direction = TurnDirection::GetTurnDir(hdgWalker, brg);
                float diff = TurnDirection::GetTurnDiff(hdgWalker, brg);

                float diffWanted = std::min(std::abs(diff), turnPerInterval);
                if( direction == TurnDirection::Dir::Left)
                    hdgWalker -= diffWanted;
                else
                    hdgWalker += diffWanted;
            }

            hdgWalker = MathSupport<float>::normAng(hdgWalker);

            walker += QVRotate(qHdgTable.Hdg(hdgWalker), Vector3F(0,0,-distInterval));

            brg = walker.bearingTo(nextStopPoint->position);
            addTmpPt(walker);

            float distToGoStright = std::abs(TurnDirection::GetTurnDiff(hdgWalker, brg))/turnPerInterval * distInterval;
            if( distToGoStright > walker.distanceTo(nextStopPoint->position))
                break;
        }

        walker += QVRotate(qHdgTable.Hdg(hdgWalker), Vector3F(0,0,-distInterval));

    } while( currentStopPointIndex > 0 && currentStopPointIndex < _ids.size()-1);

    std::shared_ptr<StopPoint> nextStopPoint = line->getStopPoint(_ids[currentStopPointIndex]);
    addTmpPt(walker, nextStopPoint);

    if( bReverse)
        std::reverse(tmpTrack.begin(), tmpTrack.end());

    int offSet = (_offset > 0) ? _offset : line->getOffset();

    Vector3F vOffSet(0,0,-offSet);

    for( int i= 0; i < tmpTrack.size(); ++i)
    {
        auto& trkPt = tmpTrack[i];

        if( i < tmpTrack.size()-1)
        {
            auto& trkPt1 = tmpTrack[i+1];

            int hdg = trkPt.pos.bearingTo(trkPt1.pos);
            hdg = MathSupport<float>::normAng(hdg);
            trkPt.hdg = hdg;
        }
        else
            trkPt.hdg = tmpTrack[i-1].hdg;

        trkPt.pos += QVRotate(qHdgTable.Hdg(trkPt.hdg-90), vOffSet);
    }

    return tmpTrack;
}
