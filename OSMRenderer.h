#ifndef OSMRENDERER_H
#define OSMRENDERER_H

#include "TFLRenderer.h"
#include "OSMData.h"
#include <QObject>

class TFLView;

class OSMRenderer : public QObject, public TFLRenderer
{
    Q_OBJECT
public:

    enum DataType
    {
        Water,
        Aeroway,
        AeroRunway,
        MotorWay,
        Primary,
        Secondary,
        Tertiary,
        Residential,
        Footway,
        Cycleway,
        Pedestrian,
    };

    OSMRenderer(const WAYPOINTS_TILES& wayPoints);

    void init(DataType type);

    QString type() const;

    void clear();
    void paint(const ViewState& viewState, QPainter& p) override;
    void paintText(const ViewState& viewState, QPainter& p) override;

    void updateCache(const ViewState& viewState) override;
    void updateCacheText(const ViewState &viewState);

    void setVisible(bool);
    bool isVisible() const;

    float getPixelsPerMile() const;

    int getNumberOfPts() const;
    qint64 getMsCacheTime() const;
    qint64 getMsRenderTime() const;

protected:

    struct osmTagCacheItem
    {
        QPoint pt;
        int length;
        WAYPOINTS* wayPoint;

        osmTagCacheItem(const QPoint& p, int len, WAYPOINTS* wp)
            : pt(p), length(len), wayPoint(wp) {}
    };

    QVector<QVector<QPoint>> _osmPts;
    std::vector<osmTagCacheItem> _osmTagCache;

    const WAYPOINTS_TILES& _wayPointTiles;
    QColor _dayColor = Qt::white;
    QColor _nightColor = Qt::white;
    qreal _lineThickness = 5.0;
    bool _visible = true;
    float _pixelPerMile = 1.0f;
    QString _tileType;

    qint64 _msCacheTime = 0;
    qint64 _msRenderTime = 0;
};

#endif // OSMRENDERER_H
