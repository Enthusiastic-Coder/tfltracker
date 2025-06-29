#pragma once

#include <jibbs/gps/GPSLocation.h>
#include <QString>
#include <QVector>

enum ModeType
{
    ModeType_Unknown,
    ModeType_Tube,
    ModeType_Bus
};

inline ModeType VehicleTypeFromMode(const QString& mode)
{
    if( mode == "tube")
        return ModeType_Tube;

    if( mode == "bus")
        return ModeType_Bus;

    return ModeType_Unknown;
}

struct Station
{
    QString name;
    GPSLocation location;
    ModeType type;
};

inline void cleanStationName(QString& name)
{
    static const QVector<QPair<QString, QString>> replacements =
    {
         {QLatin1String(" Underground Station"), QLatin1String("")},
         {QLatin1String(" DLR Station"), QLatin1String("")},
         {QLatin1String(" Tram Stop"), QLatin1String("")},
         {QLatin1String(" CROSSRAIL"), QLatin1String("")},
         {QLatin1String(" (CROSSRAIL)"), QLatin1String("")},
         {QLatin1String(" Rail Station"), QLatin1String("")},
         {QLatin1String("Terminal "), QLatin1String("T")},
         {QLatin1String("Terminals "), QLatin1String("T")},
    };

    for (const auto &pair : replacements)
    {
        name.replace(pair.first, pair.second);
    }
}


