#ifndef MAPRENDERWORKER_H
#define MAPRENDERWORKER_H

#include <QObject>
#include <QImage>
#include <QByteArray>

#include "ViewState.h"

class MapRenderer;
class TFLLineRenderer;
class TFLView;

class MapRenderWorker : public QObject
{
    Q_OBJECT
public:
    explicit MapRenderWorker(MapRenderer *mapRenderer, TFLLineRenderer *lineRenderer, QObject *parent = nullptr);

    void renderImage(ViewState snapShot, QByteArray hash);

signals:
    void imageRendered(const QImage image, const QByteArray hash);

private:
    MapRenderer* _osmRenderer;
    TFLLineRenderer* _lineRenderer;
};

#endif // MAPRENDERWORKER_H
