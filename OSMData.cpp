#include <jibbs/math/qhdgtable.h>

#include <QFile>
#include <QDebug>
#include <QDataStream>
#include <QElapsedTimer>

#include "OSMData.h"

namespace {

void initEntryPt(WAYPOINTS_TILES& name)
{
    name.setBounds(GPSLocation(51.69344,-0.511482), GPSLocation(51.28554,0.335437), 40);
}

void setViewBoundaryLimit(WAYPOINTS_TILES& name,std::pair<GPSLocation,GPSLocation>& limit)
{
    //name.setViewBoundary(limit.first, limit.second);
}

}

OSMData::OSMData()
{
    initEntryPt(_osmMotorway);
    initEntryPt(_osmPrimary);
    initEntryPt(_osmSecondary);
    initEntryPt(_osmTertiary);
    initEntryPt(_osmResidential);
    initEntryPt(_osmFootway);
    initEntryPt(_osmWater);
    initEntryPt(_osmAeroway);
    initEntryPt(_osmAeroRunway);
    initEntryPt(_osmCycleway);
    initEntryPt(_osmPedestrian);

    auto& qHdg = QHdgTable::get();
    _topLeft = qHdg.Hdg(315);
    _bottomRight = qHdg.Hdg(135);
}

std::pair<GPSLocation, GPSLocation> OSMData::adjustRange(const GPSLocation &middle, int distance) const
{
    std::pair<GPSLocation,GPSLocation> b;
    Vector4F range(0,0,-distance);
    b.first = middle + QVRotate(_topLeft, range);
    b.second = middle+ QVRotate(_bottomRight, range);
    return b;
}

void OSMData::setViewBoundary(std::pair<GPSLocation,GPSLocation> b)
{
    GPSLocation middle = (b.first + b.second)*0.5;

    double dist = b.first.distanceTo(b.second);

    if( dist < 3000 )
        b = adjustRange(middle, 3000);

    setViewBoundaryLimit(_osmAeroway, b);
    setViewBoundaryLimit(_osmAeroRunway, b);
    setViewBoundaryLimit(_osmWater, b);

    if( dist < 1000 )
        b = adjustRange(middle, 1000);

    setViewBoundaryLimit(_osmMotorway, b);
    setViewBoundaryLimit(_osmPrimary, b);
    setViewBoundaryLimit(_osmSecondary, b);

    if( dist < 500 )
        b = adjustRange(middle, 500);

    setViewBoundaryLimit(_osmResidential, b);

    setViewBoundaryLimit(_osmTertiary, b);

    setViewBoundaryLimit(_osmPedestrian, b);

    if( dist < 100 )
        b = adjustRange(middle, 100);

    setViewBoundaryLimit(_osmFootway, b);
    setViewBoundaryLimit(_osmCycleway, b);
}

void OSMData::import(const QString &filename, WAYPOINTS_TILES &tiles, bool bAllowPoints)
{
    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    if(!file.isOpen())
    {
        qDebug() << file.fileName() << ": OSM FILE NOT FOUND!";
        return;
    }

    QElapsedTimer timer;
    timer.start();

    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_6_5); // Ensure version consistency
    QByteArray blob;
    in >> blob; // Deserialize QByteArray

    QDataStream stream(&blob, QIODevice::ReadOnly);

    qsizetype count;
    stream >> count;

    for(qsizetype i = 0; i < count; ++i)
    {
        WAYPOINTS* wp(new WAYPOINTS);

        qsizetype tagCount;
        stream >> tagCount;

        wp->tags.resize(tagCount);

        for(qsizetype i = 0; i < tagCount; ++i)
        {
            QByteArray buffer;
            stream >> buffer;
            wp->tags[i].second = buffer;
            wp->tags[i].first = wp->tags[i].second.length();
        }

        qsizetype ptsCount;
        stream >> ptsCount;

        wp->gpsPts.resize(ptsCount);
        if( ptsCount > 1)
        {
            wp->bearings.resize(ptsCount-1);
        }

        double avgLat={}, avgLng={};

        for(qsizetype i = 0; i < ptsCount; ++i)
        {
            double lat;
            double lng;

            stream >> lat >> lng;

            wp->gpsPts[i]._lat = lat;
            wp->gpsPts[i]._lng = lng;

            if( i == 0)
            {
                avgLat = lat;
                avgLng = lng;
            }
            else
            {
                avgLat += lat;
                avgLng += lng;
            }
        }

        avgLat /= ptsCount;
        avgLng /= ptsCount;

        for(qsizetype i = 0; i < ptsCount-1; ++i)
            stream >> wp->bearings[i];

        if( ptsCount > 1 || bAllowPoints)
            tiles.add(wp, {avgLat, avgLng});
    }

    qDebug() << "Import [" << filename << "] -> " << timer.elapsed() << " milliseconds";
}
