#ifndef MAPRENDERER_H
#define MAPRENDERER_H

#include <jibbs/utilities/ISettingsPersist.h>

#include "OSMRenderer.h"
#include "OSMData.h"
#include <vector>

class MapRenderer : public TFLRenderer, public ISettingsPersist
{
public:
    QString debugCount() const;

    void init();
    void updateCache(const ViewState& viewState) override;
    void paint(const ViewState& viewState, QPainter& p) override;
    void paintText(const ViewState& viewState, QPainter& p) override;

    void setOSMData(std::shared_ptr<OSMData> data);
    void setViewBoundary(std::pair<GPSLocation,GPSLocation> b);

    void setMotorwayVisible(bool);
    void setPrimaryVisible(bool);
    void setSecondaryVisible(bool);
    void setTertiaryVisible(bool);
    void setResidentialVisible(bool);
    void setFootwayVisible(bool);
    void setWaterVisible(bool);
    void setAerowayVisible(bool);
    void setCycleWayVisible(bool);
    void setPedestrianVisible(bool);

    bool isMotorwayVisible() const;
    bool isPrimaryVisible() const;
    bool isSecondaryVisible() const;
    bool isTertiaryVisible() const;
    bool isResidentialVisible() const;
    bool isFootwayVisible() const;
    bool isWaterVisible() const;
    bool isAerowayVisible() const;
    bool isCycleWayVisible() const;
    bool isPedestrianVisible() const;

    void saveSettings() override;
    void loadSettings() override;

protected:
    void addRenderer(const QString& name, std::shared_ptr<OSMRenderer> &memberPtr, OSMRenderer::DataType type, const WAYPOINTS_TILES& wp);

private:

    std::shared_ptr<OSMData> _osmData;

    std::vector<OSMRenderer*> _renderObjects;

    std::shared_ptr<OSMRenderer> _motorway;
    std::shared_ptr<OSMRenderer> _secondary;
    std::shared_ptr<OSMRenderer> _tertiary;
    std::shared_ptr<OSMRenderer> _primary;
    std::shared_ptr<OSMRenderer> _residential;
    std::shared_ptr<OSMRenderer> _footway;
    std::shared_ptr<OSMRenderer> _water;
    std::shared_ptr<OSMRenderer> _aeroway;
    std::shared_ptr<OSMRenderer> _aerorunway;
    std::shared_ptr<OSMRenderer> _cycleWay;
    std::shared_ptr<OSMRenderer> _pedestrian;
};

#endif // MAPRENDERER_H
