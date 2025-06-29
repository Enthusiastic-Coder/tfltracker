#ifndef TILEMANAGER_H
#define TILEMANAGER_H

#include <jibbs/gps/GPSLocation.h>

#include <QString>
#include <QImage>

#include <map>

class TileManager
{
public:
    struct spec {

        float zoomLevel_X = 0;
        float zoomLevel_Y = 0;
        int index_x = 0;
        int index_y = 0;
    };

    struct dims {
        int x = 0;
        int y = 0;
    };

    struct zoom {
        dims dims;
        float pixPerMileX = 0;
        float pixPerMileY = 0;

        bool operator<(const zoom& rhs) const
        {
            return pixPerMileX < rhs.pixPerMileX;
        }

        bool operator==(const zoom& rhs) const
        {
            return pixPerMileX == rhs.pixPerMileX;
        }
    };

    bool isReady() const;
    bool setFolder(QString rootDir);    
    void setZoomLevel(float mapLevel);
    spec getIndex(const GPSLocation& loc);
    dims getDims(const GPSLocation &loc, int zoomlevel);

    QString getFilename(int index_x, int index_y, bool isNight);

    GPSLocation getCenterLocation(int index_x, int index_y);
    GPSLocation bottomRight();
    GPSLocation topLeft();

private:
    bool _isReady = false;
    QString _rootDir;
    GPSLocation _topLeft;
    GPSLocation _bottomRight;
    zoom _currentZoom;

    std::map<float,zoom> _levels;
};

#endif // TILEMANAGER_H
