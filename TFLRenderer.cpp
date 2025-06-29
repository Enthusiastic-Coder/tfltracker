#include "TFLRenderer.h"

void TFLRenderer::resetDirty()
{
    _bDirty = false;
}

void TFLRenderer::setDirty()
{
    _bDirty = true;
}

bool TFLRenderer::isDirty() const
{
    return _bDirty;
}

void TFLRenderer::paintHashCollection(QPainter &p, QHash<QRgb, QVector<QPoint> > &hash, int alpha, float width)
{
    QHash<QRgb, QVector<QPoint>>::iterator i;
    QPen pen;
    for(i = hash.begin(); i != hash.end(); ++i)
    {
        QColor c = i.key();
        c.setAlpha(alpha);

        pen.setColor(c);
        pen.setWidth(width);

        p.setPen(pen);
        p.drawLines(i.value());
    }
}

void TFLRenderer::paintHashCollection(QPainter &p, QHash<QRgb, QVector<QVector<QPoint> > > &hash, int alpha, float width)
{
    QHash<QRgb, QVector<QVector<QPoint>>>::iterator i;
    QPen pen;
    for(i = hash.begin(); i != hash.end(); ++i)
    {
        QColor c(i.key());
        c.setAlpha(alpha);

        pen.setWidthF(width);
        pen.setColor(c);

        p.setPen(pen);

        for( const auto& pts : i.value())
            p.drawPolyline(pts);
        //            p.drawPoints(pts);
    }
}
