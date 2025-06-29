#include <jibbs/math/qhdgtable.h>

#include "TFLLineRenderer.h"
#include "TFLView.h"
#include "WorldModel.h"
#include "Line.h"
#include "TFLModel.h"
#include "Branch.h"

#include "screenUtils.h"

const Vector3F TFLLineRenderer::static_offsets[3] = {
    {0,0,-10.0f},
    {-5,0,-5.0f},
    {5,0,-5.0f}
};

const Vector3F TFLLineRenderer::static_busstop_offsets[3] = {
    {0,0, -1.0f},
    {-0.25f,0,0.0f},
    {0.25f,0,0.0f}
};

QSet<QString> TFLLineRenderer::getStnIdFromPoint(const QPoint &p)
{
    QSet<QString> ids;
    QVector<QPair<QString, int>> candidates;

    for(auto it = _idKeyPressHitCache.begin(); it != _idKeyPressHitCache.end(); ++it)
    {
        QPoint diffPt = p - it.value();
        int manhattanDistance = diffPt.manhattanLength();
        if( manhattanDistance < 50)
            candidates << qMakePair(it.key().first, manhattanDistance);
    }
    std::sort(candidates.begin(), candidates.end(), [](const QPair<QString,int>& l, const QPair<QString,int>& r)
    {
        return l.second < r.second;
    });

    for(const auto& stn : std::as_const(candidates))
        ids << stn.first;
    return ids;
}

QSet<QString> TFLLineRenderer::getNatStnNameFromPoint(const QPoint &p)
{
    QSet<QString> ids;
    QVector<QPair<QString, int>> candidates;

    for(auto it = _idNKeyPressHitCache.begin(); it != _idNKeyPressHitCache.end(); ++it)
    {
        QPoint diffPt = p - it.value();
        int manhattanDistance = diffPt.manhattanLength();
        if( manhattanDistance < 50)
            candidates << qMakePair(it.key().second, manhattanDistance);
    }
    std::sort(candidates.begin(), candidates.end(), [](const QPair<QString,int>& l,const QPair<QString,int>& r)
    {
        return l.second < r.second;
    });

    for(const auto& stn : std::as_const(candidates))
        ids << stn.first;

    return ids;
}

void TFLLineRenderer::init(const QHash<LineType::mode, QImage>& logos)
{
    _logos = logos;
}

void TFLLineRenderer::updateCache(const ViewState &viewState)
{
    setDirty();
}

void TFLLineRenderer::doUpdateCache(const ViewState &viewState)
{
    _idNKeyPressHitCache.clear();
    _idKeyPressHitCache.clear();

    _stnTextCache.clear();

    _mapPts.clear();
    _stnLogoPtsHash.clear();
    _busStopCache.clear();
    _busPoints.clear();
    _coloredBusPoints.clear();

    const auto lines = viewState.world->getLines();
    const float PIXM = viewState.pixelsPerMile;
    const float factor = std::clamp(PIXM/25.0f, 0.1f, 50.0f);
    auto& qHdg = QHdgTable::get();

    QRect rc = viewState.boundaryFromRange;

    QSet<QString> stopsVisited;

    for(const auto& line: lines)
    {
        auto type = line->getType();

        QColor lineColor;
        if( viewState.mapNight && line->id() == Line::northernLine)
            lineColor = Qt::white;
        else
            lineColor = line->getColor();

        auto& coloredBusPts = _coloredBusPoints[lineColor.rgb()];

        const QImage& img = imgType(type);
        _stnLogoHash[type] = img;

        const int imgW2 = img.width();
        const int imgH2 = img.height()/2;

        bool showLine = line->isVisible();

        if(line->isBus())
            showLine &= _busLinesVisible;

        else if( line->isTrain())
            showLine &= _tubeLinesVisible;

        if( showLine )
        {
            auto& ptsVec = _mapPts[lineColor.rgb()];

            const Branches& branches = line->getBranches();

            for( const auto& branch : branches)
            {
                QVector<QPoint> pts;

                const auto& gpsSmoothPoints = branch->getSmoothPoints();

                if( !gpsSmoothPoints.empty() && viewState.pixelsPerMile > 65)
                {
                    for( const auto& gpsPt : gpsSmoothPoints)
                    {
                        QPoint pt = screenUtils::toScreen(viewState, gpsPt->position);

                        if( rc.contains(pt) || screenUtils::ptInScreen(viewState, gpsPt->position))
                        {
                            pts << pt;
                        }
                        else if( !pts.empty())
                        {
                            ptsVec << pts;
                            pts.clear();
                        }
                    }
                }
                else
                {
                    const auto& gpsPoints = branch->getStnPoints();

                    for( const auto& gpsPt : gpsPoints)
                        pts << screenUtils::toScreen(viewState, gpsPt);
                }

                ptsVec << pts;

                if(line->getShowStops() && line->isBus() && PIXM > 65)
                {
                    const auto& points = branch->getSmoothStnPoints();

                    for( const auto& point : points)
                    {
                        QPoint pt = screenUtils::toScreen(viewState, point.pos);

                        if( rc.contains(pt) || screenUtils::ptInScreen(viewState, point.pos))
                        {
                            QPolygonF polyF;

                            auto q = qHdg.Hdg(point.hdg);

                            for(int i=0; i < 3; ++i)
                            {
                                Vector3F v = QVRotate(q, static_busstop_offsets[i]);
                                polyF << pt + QPointF(v.x*factor, v.z*factor);
                            }

                            coloredBusPts << polyF;
                        }
                    }
                }
            }
        }

        if( line->isBus() && !_busStopVisible)
            continue;

        if( (line->isBus() && PIXM < 250.0f) )
            continue;

        if( viewState.pixelsPerMile < 100 )
            continue;

        const bool drawStnText = (!line->isBus() && viewState.pixelsPerMile > 250);

        const auto& stopPoints = line->getStopPoints();

        const int offSetDist = line->isNationalRail() ? 30 : (line->isElizabethRail() ?60:0);

        const QPoint offset(offSetDist,0);
        const QPoint imgOffSet(imgW2,imgH2);
        auto& stnLogoPts = _stnLogoPtsHash[type];

        for(const auto& stopPoint : stopPoints)
        {
            if( !stopPoint.second->visible)
                continue;

            if( stopPoint.second->isPassPoint)
            {
                continue;
            }

            const QPoint originalScreenPosition =  screenUtils::toScreen(viewState, stopPoint.second->position);

            if( !screenUtils::ptInScreen(viewState, originalScreenPosition))
                continue;

            GPSLocation adjustedPosition = stopPoint.second->position + Vector3F(offSetDist,0,0);

            QPoint pt = screenUtils::toScreen(viewState, adjustedPosition);

            if(drawStnText)
                _stnTextCache[stopPoint.second->displayName] = pt + imgOffSet;

            bool addToKeyPressCache(true);

            if( line->isBus())
            {
                if( viewState.pixelsPerMile > 1000 )
                {
                    BusStopCacheEntry e;
                    e.pt = pt;

                    e.letter = stopPoint.second->stopLetter;
                    if( e.letter.isEmpty())
                         e.letter = QStringLiteral("-");

                    e.lastStop = stopPoint.second->order == stopPoints.size() - 1;

                    auto q = qHdg.Hdg(stopPoint.second->bearing);

                    for(int i=0; i < 3; ++i)
                    {
                        Vector3F v = QVRotate(q, static_offsets[i]);
                        e.dir[i].setX(v.x);
                        e.dir[i].setY(v.z);
                    }

                    _busStopCache[stopPoint.second->id] = e;
                }
                else if( viewState.pixelsPerMile > 500)
                {
                    if( stopsVisited.find(stopPoint.second->id) == stopsVisited.end())
                    {
                        stopsVisited.insert(stopPoint.second->id);
                        stnLogoPts << pt;
                    }
                }
                else
                {
                    if( stopsVisited.find(stopPoint.second->id) == stopsVisited.end())
                    {
                        stopsVisited.insert(stopPoint.second->id);
                        _busPoints << pt;
                    }
                    addToKeyPressCache = false;
                }
            }
            else
            {
                if(line->isNationalRail()|| line->isOverground() || line->isElizabethRail())
                    _idNKeyPressHitCache[qMakePair(stopPoint.second->id, stopPoint.second->displayName)] = pt;

                if( !stopPoint.second->isPassPoint)
                {
                    stnLogoPts << pt;
                }
            }

            if( addToKeyPressCache && !stopPoint.second->isPassPoint)
                _idKeyPressHitCache[qMakePair(stopPoint.second->id, QString::fromStdString(adjustedPosition.toString()))] = pt;
        }
    }
}

void TFLLineRenderer::paint(const ViewState &viewState, QPainter &p)
{
    if(isDirty())
    {
        resetDirty();
        doUpdateCache(viewState);
    }

    paintHashCollection( p, _mapPts, 255, 2 );

    QPen oldPen = p.pen();

    QPen pen;
    pen.setColor(Qt::red);
    pen.setWidth(2);
    p.setPen(pen);

    p.drawPoints(_busPoints);

    p.setPen(oldPen);

    paintCustomHashCollection(p, _coloredBusPoints, [&p](const QVector<QPolygonF>& pts){

        for(const auto& poly : pts)
            p.drawPolygon(poly);
    });


    paintStnImage(viewState, p, _stnLogoPtsHash);
}

void TFLLineRenderer::paintText(const ViewState &viewState, QPainter &p)
{
    QFont f("Verdana");
    f.setPixelSize( viewState.pixelsPerMile > 1000? 14 : 10);
    f.setBold(true);
    p.setFont(f);

    QTransform old = p.transform();
    QPen oldPen = p.pen();
    p.setPen(viewState.mapNight? "#FFFFFF": "#008FCE");

    for(auto it = _stnTextCache.begin(); it != _stnTextCache.end(); ++it)
    {
        p.translate(it.value()-QPoint(20,0));
        p.rotate(viewState.compassValue);
        p.translate(-it.value()+QPoint(20,0));
        p.drawText(it.value(), it.key());
        p.setTransform(old);
    }

    p.setPen( Qt::white);

    QFontMetrics fm = p.fontMetrics();
    QSize sz;
    sz.setHeight(fm.height());
    QPen busStop;
    busStop.setColor(viewState.mapNight?Qt::white:Qt::black);

    QTextOption to;
    to.setAlignment(Qt::AlignCenter|Qt::AlignHCenter);

    QPen redPen(Qt::red);

    for(auto it = _busStopCache.begin();  it != _busStopCache.end(); ++it)
    {
        if( it->second.lastStop)
            continue;

        sz.setWidth(fm.horizontalAdvance(it->second.letter));
        QPoint pt( it->second.pt.x(), it->second.pt.y());
        int dim = qMax(sz.width(), sz.height());

        p.setBrush(Qt::red);

        redPen.setWidthF(2.0f);
        p.setPen(redPen);

        const float fFactor = dim*1.0f/10.0f;

        QVector<QPointF> lines;
        QPointF head = pt + it->second.dir[0] * fFactor;

        lines << pt << pt + it->second.dir[0] * fFactor;
        lines << head << pt + it->second.dir[1] * fFactor;
        lines << head << pt + it->second.dir[2] * fFactor;

        p.drawLines(lines);
    }

    for(auto it = _busStopCache.begin();  it != _busStopCache.end(); ++it)
    {
        sz.setWidth(fm.horizontalAdvance(it->second.letter));
        QPoint pt( it->second.pt.x(), it->second.pt.y());
        int dim = qMax(sz.width(), sz.height());
        QRect rc(pt.x()-dim/2, pt.y()-dim/2, dim, dim);

        p.setBrush(Qt::red);

        p.translate(it->second.pt);
        p.rotate(viewState.compassValue);
        p.translate(-it->second.pt);

        redPen.setWidthF(1.0f);
        p.setPen(redPen);

        p.drawEllipse(rc);
        p.setBrush(Qt::NoBrush);

        p.setPen(Qt::white);
        p.drawText(rc, it->second.letter, to);

        p.setTransform(old);
    }

    p.setPen(oldPen);
}

void TFLLineRenderer::setBusStopVisible(bool b)
{
    _busStopVisible = b;
}

bool TFLLineRenderer::isBusStopVisible() const
{
    return _busStopVisible;
}

void TFLLineRenderer::setBusLinesVisible(bool b)
{
    _busLinesVisible = b;
}

bool TFLLineRenderer::isBusLinesVisible() const
{
    return _busLinesVisible;
}

void TFLLineRenderer::setTubeLinesVisible(bool b)
{
    _tubeLinesVisible = b;
}

bool TFLLineRenderer::isTubeLinesVisible() const
{
    return _tubeLinesVisible;
}

void TFLLineRenderer::paintStnImage(const ViewState &viewState, QPainter &p, QHash<LineType::mode, QVector<QPoint> > &hash)
{
    QTransform old = p.transform();

    for(auto i = hash.begin(); i != hash.end(); ++i)
    {
        const QImage& img = _stnLogoHash[i.key()];

        QPoint offSet(img.width()/2, img.height()/2);

        for( const auto& pt : i.value())
        {
            p.translate(pt);
            p.rotate(viewState.compassValue);
            p.translate(-pt);

            p.drawImage( pt - offSet, img);
            p.setTransform(old);
        }
    }
}

const QImage &TFLLineRenderer::imgType(LineType::mode type) const
{
    auto it = _logos.constFind(type);
    if( it == _logos.cend())
    {
        static QImage img;
        return img;
    }

    return it.value();
}

