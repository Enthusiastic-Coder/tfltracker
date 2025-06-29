#ifndef TFLRENDERER_H
#define TFLRENDERER_H

#include <QPainter>
#include <QHash>

#include "ViewState.h"

class TFLView;

class TFLRenderer
{
public:
    virtual ~TFLRenderer() = default;

    virtual void updateCache(const ViewState& viewState) = 0;
    virtual void paint(const ViewState& viewState, QPainter& p) = 0;
    virtual void paintText(const ViewState& viewState, QPainter& p) = 0;

    void resetDirty();
    void setDirty();
    bool isDirty() const;

protected:
    template<typename T,typename U>
    void paintCustomHashCollection(QPainter &p, T &hash, const U& impl)
    {
        QPen pen;
        for(auto i = hash.begin(); i != hash.end(); ++i)
        {
            QColor c = i.key();

            pen.setColor(c);
            p.setPen(pen);
            p.setBrush(c);

            impl(i.value());
        }
    }

    void paintHashCollection(QPainter& p, QHash<QRgb, QVector<QPoint> > &hash, int alpha, float width);
    void paintHashCollection(QPainter& p, QHash<QRgb, QVector<QVector<QPoint> > > &hash, int alpha, float width);


private:
    bool _bDirty = false;
};

#endif // TFLRENDERER_H
