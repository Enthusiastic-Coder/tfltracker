#include "MapRenderer.h"
#include <QSettings>

QString MapRenderer::debugCount() const
{
    QString txt;

    for( auto& renderObject : _renderObjects)
    {
        txt.append( QString("[%1:%2:%3:%4]")
                       .arg(renderObject->type().left(5))
                       .arg(renderObject->getNumberOfPts())
                       .arg(renderObject->getMsCacheTime())
                       .arg(renderObject->getMsRenderTime()));
    }

    return txt;
}

void MapRenderer::addRenderer(const QString &name, std::shared_ptr<OSMRenderer> &memberPtr, OSMRenderer::DataType type, const WAYPOINTS_TILES &wp)
{
    memberPtr = std::make_shared<OSMRenderer>(wp);
    memberPtr->init(type);
    memberPtr->setObjectName("OSMRender" + name);
    _renderObjects.push_back(memberPtr.get());
}

void MapRenderer::init()
{
    addRenderer("AeroWay", _aeroway, OSMRenderer::Aeroway, _osmData->getAeroWay());
    addRenderer("AeroRunway", _aerorunway, OSMRenderer::AeroRunway, _osmData->getAeroRunway());
    addRenderer("Water", _water, OSMRenderer::Water, _osmData->getWater());
    addRenderer("Footway", _footway, OSMRenderer::Footway, _osmData->getFootway());
    addRenderer("CycleWay", _cycleWay, OSMRenderer::Cycleway, _osmData->getCycleWay());
    addRenderer("Pedestrian", _pedestrian, OSMRenderer::Pedestrian, _osmData->getPedestrian());
    addRenderer("Tertiary", _tertiary, OSMRenderer::Tertiary, _osmData->getTertiary());
    addRenderer("Residential", _residential,OSMRenderer::Residential, _osmData->getResidential());
    addRenderer("Secondary", _secondary, OSMRenderer::Secondary, _osmData->getSecondary());
    addRenderer("Primary", _primary, OSMRenderer::Primary, _osmData->getPrimary());
    addRenderer("MotorWay", _motorway, OSMRenderer::MotorWay, _osmData->getMotorway());

    _footway->setVisible(false);
}

void MapRenderer::updateCache(const ViewState &viewState)
{
    if( !isDirty())
    {
        return;
    }

    resetDirty();

    for( auto& renderObject : _renderObjects)
    {
        renderObject->clear();

        if( viewState.pixelsPerMile <  renderObject->getPixelsPerMile())
            continue;

        renderObject->updateCache(viewState);
        renderObject->updateCacheText(viewState);
    }
}

void MapRenderer::paint(const ViewState &viewState, QPainter &p)
{
    updateCache(viewState);

    for( auto& renderObject : _renderObjects)
    {
        if( viewState.pixelsPerMile < renderObject->getPixelsPerMile())
            continue;

        renderObject->paint(viewState, p);
    }
}

void MapRenderer::paintText(const ViewState &viewState, QPainter &p)
{
    updateCache(viewState);

    for( auto& renderObject : _renderObjects)
        renderObject->paintText(viewState, p);
}

void MapRenderer::setOSMData(std::shared_ptr<OSMData> data)
{
    _osmData = data;
}

void MapRenderer::setViewBoundary(std::pair<GPSLocation, GPSLocation> b)
{
    _osmData->setViewBoundary(b);
}

void MapRenderer::setMotorwayVisible(bool b)
{
    _motorway->setVisible(b);
}

void MapRenderer::setPrimaryVisible(bool b)
{
    _primary->setVisible(b);
}

void MapRenderer::setSecondaryVisible(bool b)
{
    _secondary->setVisible(b);
}

void MapRenderer::setTertiaryVisible(bool b)
{
    _tertiary->setVisible(b);
}

void MapRenderer::setResidentialVisible(bool b)
{
    _residential->setVisible(b);
}

void MapRenderer::setFootwayVisible(bool b)
{
    _footway->setVisible(b);
}

void MapRenderer::setWaterVisible(bool b)
{
    _water->setVisible(b);
}

void MapRenderer::setAerowayVisible(bool b)
{
    _aeroway->setVisible(b);
    _aerorunway->setVisible(b);
}

void MapRenderer::setCycleWayVisible(bool b)
{
    _cycleWay->setVisible(b);
}

void MapRenderer::setPedestrianVisible(bool b)
{
    _pedestrian->setVisible(b);
}

bool MapRenderer::isMotorwayVisible() const
{
    return _motorway->isVisible();
}

bool MapRenderer::isPrimaryVisible() const
{
    return _primary->isVisible();
}

bool MapRenderer::isSecondaryVisible() const
{
    return _secondary->isVisible();
}

bool MapRenderer::isTertiaryVisible() const
{
    return _tertiary->isVisible();
}

bool MapRenderer::isResidentialVisible() const
{
    return _residential->isVisible();
}

bool MapRenderer::isFootwayVisible() const
{
    return _footway->isVisible();
}

bool MapRenderer::isWaterVisible() const
{
    return _water->isVisible();
}

bool MapRenderer::isAerowayVisible() const
{
    return _aeroway->isVisible();
}

bool MapRenderer::isCycleWayVisible() const
{
    return _cycleWay->isVisible();
}

bool MapRenderer::isPedestrianVisible() const
{
    return _pedestrian->isVisible();
}

void MapRenderer::saveSettings()
{
    QSettings s;

    for( auto& renderObject : _renderObjects)
    {
        s.setValue("view/OSM/" + renderObject->objectName(), renderObject->isVisible());
    }
}

void MapRenderer::loadSettings()
{
    QSettings s;

    for( auto& renderObject : _renderObjects)
    {
        renderObject->setVisible(s.value("view/OSM/" + renderObject->objectName(), renderObject->isVisible()).toBool());
    }
}
