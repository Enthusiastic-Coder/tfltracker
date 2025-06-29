#ifndef SCREENUTILS_H
#define SCREENUTILS_H

#include "ViewState.h"
#include <QPoint>
#include <jibbs/gps/GPSLocation.h>


namespace screenUtils {

static QPoint toScreen(const ViewState &snapshot, const GPSLocation &location, bool bApplyTransform =false)
{
    const QSize& sz = snapshot.middleOfScreen;
    QPoint center(sz.width(), sz.height());

    Vector3F d = snapshot.gpsOrigin.offSetTo(location) / 1609.334f * snapshot.pixelsPerMile;

    QPoint ptDiff(d.x, d.z);
    QPoint ptRot;

    if (bApplyTransform)
        ptRot = snapshot.invCompassTransform.map(ptDiff);
    else
        ptRot = ptDiff;

    return center + ptRot;
}

static bool ptInScreen(const ViewState &snapshot, const QPoint &pt)
{
    int borderSizeX = 0.15*snapshot.size.width();
    int borderSizeY = 0.15*snapshot.size.height();

    //    if(getZoomLevel() >= TFLView::ZoomLevel::High)
    //        borderSize = width()/16;

    if (pt.x() < -borderSizeX)
        return false;

    if (pt.y() < -borderSizeY)
        return false;

    if (pt.x() > (snapshot.size.width()+borderSizeX) )
        return false;

    if (pt.y() > (snapshot.size.height()+borderSizeY))
        return false;

    return true;
}

static bool ptInScreen( const QSize &size, const QPoint &pt)
{
    int borderSizeX = 0.15*size.width();
    int borderSizeY = 0.15*size.height();

    //    if(getZoomLevel() >= TFLView::ZoomLevel::High)
    //        borderSize = width()/16;

    if (pt.x() < -borderSizeX)
        return false;

    if (pt.y() < -borderSizeY)
        return false;

    if (pt.x() > (size.width()+borderSizeX) )
        return false;

    if (pt.y() > (size.height()+borderSizeY))
        return false;

    return true;
}

static bool ptInScreen(const ViewState &snapshot, const GPSLocation &loc)
{
    return ptInScreen(snapshot, toScreen(snapshot, loc, true));
}



}


#endif // SCREENUTILS_H
