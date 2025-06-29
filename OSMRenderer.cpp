#include "OSMRenderer.h"
#include <jibbs/gps/GPSLocation.h>
#include <QDebug>

#include <QElapsedTimer>

#include "screenUtils.h"

OSMRenderer::OSMRenderer(const WAYPOINTS_TILES &wayPoints) :
    _wayPointTiles(wayPoints)
{

}

void OSMRenderer::init(DataType type)
{
    if( type == DataType::Water)
    {
        _dayColor = Qt::blue;
        _nightColor = Qt::darkCyan;
        _lineThickness = 1.0;
        _pixelPerMile = 1;
        _tileType = "Water";
    }
    else if( type == DataType::Aeroway)
    {
        _dayColor = Qt::lightGray;
        _nightColor = Qt::lightGray;
        _lineThickness = 2.0;
        _pixelPerMile = 1;
        _tileType = "Aeroway";
    }
    else if( type == DataType::AeroRunway)
    {
        _dayColor = Qt::lightGray;
        _nightColor = Qt::white;
        _lineThickness = 15.0;
        _pixelPerMile = 1;
         _tileType = "AeroRunway";
    }
    else if( type == DataType::MotorWay)
    {
        _dayColor = QColor("#cccc00");
        _nightColor = Qt::darkYellow;
        _lineThickness = 5.0;
        _pixelPerMile = 1;
        _tileType = "MotorWay";
    }
    else if( type == DataType::Primary)
    {
        _dayColor = Qt::white;
        _nightColor = Qt::lightGray;
        _lineThickness = 5.0;
        _pixelPerMile = 100;
        _tileType = "Primary";
    }
    else if( type == DataType::Secondary)
    {
        _dayColor = QColor("#eeeeee");
        _nightColor = Qt::lightGray;
        _lineThickness = 2.0;
        _pixelPerMile = 200;
        _tileType = "Secondary";
    }
    else if( type == DataType::Tertiary)
    {
        _dayColor = QColor("#eeeeee");
        _nightColor = Qt::darkGray;
        _lineThickness = 2.0;
        _pixelPerMile = 300;
        _tileType = "Tertiary";
    }
    else if( type == DataType::Residential)
    {
        _dayColor = QColor("#eeeeee");
        _nightColor = Qt::darkGray;
        _lineThickness = 2.0;
        _pixelPerMile = 300;
        _tileType = "Residential";
    }
    else if( type == DataType::Footway)
    {
        _dayColor = Qt::lightGray;
        _nightColor = Qt::darkGray;
        _lineThickness = 1.0;
        _pixelPerMile = 300;
        _tileType = "Footway";
    }
    else if( type == DataType::Cycleway)
    {
        _dayColor = Qt::lightGray;
        _nightColor = Qt::darkGray;
        _lineThickness = 1.0;
        _pixelPerMile = 400;
        _tileType = "Cycleway";
    }
    else if( type == DataType::Pedestrian)
    {
        _dayColor = Qt::white;
        _nightColor = Qt::darkGray;
        _lineThickness = 2.0;
        _pixelPerMile = 400;
        _tileType = "Pedestrian";
    }
}

QString OSMRenderer::type() const
{
    return _tileType;
}

void OSMRenderer::clear()
{
    _osmPts.clear();
    _osmTagCache.clear();
}

void OSMRenderer::paint(const ViewState& viewState, QPainter &p)
{
    _msRenderTime = 0;

    if( !_visible)
        return;

#ifdef Q_OS_WIN
    QElapsedTimer timer;
    timer.start();
#endif

    QPen oldPen = p.pen();

    const float pixM = viewState.pixelsPerMile;

    QPen pen(viewState.mapNight? _nightColor: _dayColor);
    pen.setWidthF(qMax(_lineThickness, _lineThickness* pixM / 1000.0));
    p.setPen( pen);

    const auto& osmPts = _osmPts;

    for( const auto& pts : std::as_const(osmPts))
        p.drawPolyline(pts);

    p.setPen(oldPen);

#ifdef Q_OS_WIN
    _msRenderTime = timer.elapsed();
#endif
}

void OSMRenderer::paintText(const ViewState &viewState, QPainter &p)
{
    if( !_visible)
        return;

    QFont labelFont;
    labelFont.setFamily("Verdana");
    labelFont.setPixelSize(viewState.pixelsPerMile > 2000? 14: 10);
    p.setFont(labelFont);

    p.setPen(viewState.mapNight? Qt::white:Qt::darkGray);

    QFontMetrics fm = p.fontMetrics();

    for(const auto& tagItem : std::as_const(_osmTagCache))
    {
        if( fm.horizontalAdvance(tagItem.wayPoint->tags[0].second) > tagItem.length)
            continue;

        const QString& textToRender = tagItem.wayPoint->tags[0].second;

        if( tagItem.wayPoint->bearings.empty())
        {
            if( screenUtils::ptInScreen( viewState, tagItem.pt))
                p.drawText(tagItem.pt, textToRender);
        }
        else
        {
            QTransform oldT = p.transform();
            p.translate(tagItem.pt);
            float brg = tagItem.wayPoint->bearings[tagItem.wayPoint->bearings.size()/2];

            bool flip =MathSupport<float>::normAng(brg -  viewState.compassValue) > 179.0f;
            if( flip )
                p.rotate(int(brg+90));
            else
                p.rotate(int(brg-90));

            const int textWidth = fm.horizontalAdvance(textToRender);

            QRect rc;
            rc.setWidth(textWidth);
            rc.setHeight(fm.height());
            rc.setY(-rc.height());

            if( !flip)
                rc.setX(-textWidth);

            if( screenUtils::ptInScreen(viewState, QPoint(rc.left(), rc.top())))
                p.drawText(rc, textToRender);

            if( !tagItem.wayPoint->bearings.empty())
                p.setTransform(oldT);
        }
    }
}

void OSMRenderer::updateCache(const ViewState &viewState)
{
    _msCacheTime = 0;

    if( !_visible)
        return;

#ifdef Q_OS_WIN
    QElapsedTimer timer;
    timer.start();
#endif

    _osmPts.clear();

    GPSLocation lastPos;

    const auto& ids = _wayPointTiles.getViewableTiles(viewState.boundaryView.first, viewState.boundaryView.second);

    QRect rc = viewState.boundaryFromRange;

    for(const auto& id: ids)
    {
        const auto& tile = _wayPointTiles.getTile(id);

        for( const auto& item: tile)
        {
            if( item->gpsPts.size() < 2)
                continue;

            QVector<QPoint> pts;

            for( const auto& gpsPt : item->gpsPts)
            {
                QPoint pt = screenUtils::toScreen(viewState, gpsPt);

                if( rc.contains(pt) || screenUtils::ptInScreen(viewState, gpsPt))
                    pts << pt;
            }

            _osmPts << pts;
        }
    }

#ifdef Q_OS_WIN
    _msCacheTime = timer.elapsed();
#endif
}

void OSMRenderer::updateCacheText(const ViewState &viewState)
{
    if( !_visible)
        return;

    if( viewState.pixelsPerMile < 500 )
        return;

    _osmTagCache.clear();

    GPSLocation lastPos;

    auto ids = _wayPointTiles.getViewableTiles(viewState.boundaryView.first, viewState.boundaryView.second);

    for(const auto& id: ids)
    {
        const auto& tile = _wayPointTiles.getTile(id);

        for(const auto& item: tile)
        {
            QVector<QPoint> pts;

            if( item->gpsPts.size() < 2)
                continue;

            const GPSLocation& p1 = item->gpsPts[0];
            const GPSLocation& p2 = item->gpsPts[item->gpsPts.size()-1];

            QPoint a( screenUtils::toScreen(viewState, p1));
            QPoint b( screenUtils::toScreen(viewState, p2));

            if( !screenUtils::ptInScreen(viewState, a))
                continue;

            if( !screenUtils::ptInScreen(viewState, b))
                continue;

            pts << a << b;

            if( item->tags.size() > 0 && pts.size() > 1)
            {
                const QPoint& pt = pts[pts.size()/2];

                _osmTagCache.emplace_back( pt, (a-b).manhattanLength(), item);
            }
        }
    }
}

void OSMRenderer::setVisible(bool b)
{
    _visible = b;
}

bool OSMRenderer::isVisible() const
{
    return _visible;
}

float OSMRenderer::getPixelsPerMile() const
{
    return _pixelPerMile;
}

int OSMRenderer::getNumberOfPts() const
{
    return static_cast<int>(_osmPts.size()) ;
}

qint64 OSMRenderer::getMsCacheTime() const
{
    return _msCacheTime;
}

qint64 OSMRenderer::getMsRenderTime() const
{
    return _msRenderTime;
}

