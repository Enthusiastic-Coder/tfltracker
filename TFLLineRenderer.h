#ifndef TFLLINERENDERER_H
#define TFLLINERENDERER_H

#include <jibbs/vector/vector3.h>
#include <QMap>

#include <unordered_map>

#include "TFLRenderer.h"
#include "LineType.h"
#include "ViewState.h"



class TFLLineRenderer : public TFLRenderer
{
public:
    QSet<QString> getStnIdFromPoint(const QPoint& p);
    QSet<QString> getNatStnNameFromPoint(const QPoint& p);

    void init(const QHash<LineType::mode, QImage> &logos);
    void updateCache(const ViewState& viewState) override;
    void paint(const ViewState& viewState, QPainter& p) override;
    void paintText(const ViewState& viewState, QPainter& p) override;

    void setBusStopVisible(bool);
    bool isBusStopVisible() const;

    void setBusLinesVisible(bool);
    bool isBusLinesVisible() const;

    void setTubeLinesVisible(bool);
    bool isTubeLinesVisible() const;


protected:
    void paintStnImage(const ViewState &viewState, QPainter& p, QHash<LineType::mode, QVector<QPoint> > &hash);
    const QImage& imgType(LineType::mode type) const;
    void doUpdateCache(const ViewState &viewState);

private:
    static const Vector3F static_offsets[3];
    static const Vector3F static_busstop_offsets[3];

    struct BusStopCacheEntry
    {
        bool lastStop;
        QString letter;
        QPoint pt;
        QPointF dir[3];
    };

    QHash<LineType::mode, QImage> _logos;
    QHash<QPair<QString,QString>, QPoint> _idKeyPressHitCache;
    QHash<QPair<QString,QString>, QPoint> _idNKeyPressHitCache;
    QHash<QString, QPoint> _stnTextCache;
    std::unordered_map<QString,BusStopCacheEntry> _busStopCache;
    QHash<QRgb, QVector<QVector<QPoint>>> _mapPts;
    QHash<LineType::mode, QVector<QPoint>> _stnLogoPtsHash;
    QHash<LineType::mode, QImage> _stnLogoHash;
    QVector<QPoint> _busPoints;
    QHash<QRgb,QVector<QPolygonF>> _coloredBusPoints;
    bool _busStopVisible = true;
    bool _busLinesVisible = true;
    bool _tubeLinesVisible = true;
};

#endif // TFLLINERENDERER_H
