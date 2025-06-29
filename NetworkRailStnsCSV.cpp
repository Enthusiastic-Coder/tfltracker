#include "NetworkRailStnsCSV.h"


void NetworkRailStnsCSV::onLine(int lineNo, const QStringList& args)
{
    auto& item = _data[args[0]];
    item.location = args[1];
    item.stanme = args[2];
}

