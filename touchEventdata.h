#pragma once

#include <QEvent>
#include <QPoint>

struct touchEventData
{
    QEvent::Type type;
    int touchPointCounts = 0;

    QPoint singlePt;

    ///multipress
    QPointF diffPt;
    QPointF diffPt0;
    qreal currentScaleFactor;
};
