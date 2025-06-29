#include "NationalRailCRC.h"

NationalRailCRC::NationalRailCRC()
{

}

void NationalRailCRC::onLine(int lineNo, const QStringList &args)
{
    if( lineNo==0)
        return;

    if( args.count() <2)
        return;

    if( args[0] == QStringLiteral("Shepherd's Bush"))
        _stationCRC[QStringLiteral("Shepherds Bush")] = args[1];
    else
        _stationCRC[args[0]] = args[1];
}

QString NationalRailCRC::getCRCFromStnName(QString stnName) const
{
    auto it = _stationCRC.find(stnName);
    if( it == _stationCRC.end())
        return "";

    return it->second;
}
