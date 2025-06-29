#ifndef NETWORKRAILSTNSCSV_H
#define NETWORKRAILSTNSCSV_H

#include "csvfileload.h"

struct NetworkRailStnsCSVDATA {
    QString location;
    QString stanme;
};

class NetworkRailStnsCSV : public CSVFileLoad<NetworkRailStnsCSVDATA>
{
public:
    virtual void onLine(int lineNo, const QStringList& args) override;
};

#endif // NETWORKRAILSTNSCSV_H
