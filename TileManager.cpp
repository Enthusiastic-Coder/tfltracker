#include "TileManager.h"
#include <QDirIterator>
#include <QTextStream>
#include <QImage>
#include <QDebug>


bool TileManager::isReady() const
{
    return _isReady;
}

bool TileManager::setFolder(QString rootDir)
{
    _rootDir = rootDir;
    _levels.clear();

    QFile boundsFile(rootDir +"/bounds.txt");
    if( !boundsFile.open(QIODevice::ReadOnly))
        return false;

    QTextStream stream(&boundsFile);
    _topLeft = GPSLocation( stream.readLine().toStdString());
    _bottomRight = GPSLocation( stream.readLine().toStdString());
    boundsFile.close();

//    GPSLocation tileDiff = bottomRight() - topLeft();

    double distX = topLeft().distanceTo(GPSLocation(topLeft()._lat, bottomRight()._lng));
    double distY = topLeft().distanceTo(GPSLocation(bottomRight()._lat, topLeft()._lng));

    QDirIterator dirIt(rootDir, QDir::Dirs|QDir::NoDotAndDotDot);

    while( dirIt.hasNext())
    {
        dirIt.next();

        QStringList dimList = dirIt.fileName().split("x");

        int dim = 1024;

        QString fullFilename = dirIt.filePath()+"/day";
        QDirIterator dirImage(fullFilename, QDir::Files|QDir::NoDotAndDotDot);

        if( dirImage.hasNext())
        {
            dirImage.next();

            QImage img;
            if(img.load(dirImage.filePath()))
                dim = img.width();
        }

        zoom zm;
        zm.dims.x = dimList[0].toInt();
        zm.dims.y = dimList[1].toInt();

        zm.pixPerMileX = dim * zm.dims.x / distX * 1609.34;
        zm.pixPerMileY = dim * zm.dims.y / distY * 1609.34;

        _levels[zm.pixPerMileX] = zm;
    }

    _isReady = true;

    return true;
}

void TileManager::setZoomLevel(float mapLevel)
{
    _currentZoom = _levels.begin()->second;

    for( auto level : _levels)
    {
        if( mapLevel >= level.second.pixPerMileX )
        {
            _currentZoom = level.second;
        }
        else
        {
            break;
        }
    }
}

TileManager::spec TileManager::getIndex(const GPSLocation &loc)
{
    GPSLocation tileDiff = bottomRight() - topLeft();
    spec ret;

    double U = (loc._lng - topLeft()._lng) / tileDiff._lng;
    double V = (loc._lat - topLeft()._lat) / tileDiff._lat;

    ret.index_x= std::floor(_currentZoom.dims.x * U);
    ret.index_y = std::floor(_currentZoom.dims.y * V);
    ret.zoomLevel_X = _currentZoom.pixPerMileX;
    ret.zoomLevel_Y = _currentZoom.pixPerMileY;
    return ret;
}

QString TileManager::getFilename(int index_x, int index_y, bool isNight)
{
    QString filename = QString("%1/%2x%3/%4/%5_%6.png")
            .arg(_rootDir)
            .arg(_currentZoom.dims.x)
            .arg(_currentZoom.dims.y)
            .arg(isNight?"night":"day")
            .arg(index_x)
            .arg(index_y);

    return filename;
}

GPSLocation TileManager::getCenterLocation(int index_x, int index_y)
{
    if( _currentZoom.dims.x < 1 || _currentZoom.dims.y < 1)
        return GPSLocation();

    GPSLocation tileDiff = bottomRight() - topLeft();

    GPSLocation result;
    result._lng = float(index_x) / _currentZoom.dims.x * tileDiff._lng + topLeft()._lng + tileDiff._lng/2/_currentZoom.dims.x;
    result._lat = float(index_y) / _currentZoom.dims.y * tileDiff._lat + topLeft()._lat + tileDiff._lat/2 / _currentZoom.dims.y;
    return result;
}

GPSLocation TileManager::bottomRight()
{
    return _bottomRight;
}

GPSLocation TileManager::topLeft()
{
    return _topLeft;
}
