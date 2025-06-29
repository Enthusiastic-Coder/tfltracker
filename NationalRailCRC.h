#ifndef NATIONALRAILCRC_H
#define NATIONALRAILCRC_H

#include "csvfileload.h"
#include <map>

class NationalRailCRC : public CSVFileLoad<int>
{
public:
    NationalRailCRC();

    virtual void onLine(int lineNo, const QStringList& args) override;

    QString getCRCFromStnName(QString stnName) const;
private:
    std::map<QString,QString> _stationCRC;
};

#endif // NATIONALRAILCRC_H
