#include "MapRenderWorker.h"
#include "MapRenderer.h"
#include "TFLLineRenderer.h"

#include <QImage>
#include <QPainter>

MapRenderWorker::MapRenderWorker(MapRenderer *mapRenderer, TFLLineRenderer* lineRenderer, QObject *parent)
    : QObject{parent}, _osmRenderer(mapRenderer), _lineRenderer(lineRenderer)
{
}

void MapRenderWorker::renderImage(ViewState snapShot, QByteArray hash)
{
    QColor bgColor;
    QSize size = snapShot.size;
    float devicePixelRatio = 1.0f;

    QImage image(size, QImage::Format_ARGB32);
    image.fill(bgColor);

    QPainter p(&image);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.scale(devicePixelRatio, devicePixelRatio);


    _osmRenderer->paint(snapShot, p);

    _lineRenderer->paint(snapShot, p);

    _osmRenderer->paintText(snapShot, p);

    _lineRenderer->paintText(snapShot, p);

    p.end();

    emit imageRendered(image, hash);
}
