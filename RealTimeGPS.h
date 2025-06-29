#pragma once

#include <QTime>

struct RealTimeGPSInfo
{
    float nativeSpd = 0.0f;
    float spd = 0.0f;
    float alt = 0.0f;
    float vsi = 0.0f;
    float hdg = 0.0f;
    QTime lastUpdate;
};

struct RealTimeGPSNewInfo : RealTimeGPSInfo
{
    bool hasSpeed = false;
    bool hasHdg = false;
    bool hasVSI = false;
};


