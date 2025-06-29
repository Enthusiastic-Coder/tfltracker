#ifndef STOPPOINTMINS_H
#define STOPPOINTMINS_H

#include "csvfileload.h"
#include <map>

class StopPointMins : public CSVFileLoad<int>
{
public:
    virtual void onLine(int lineNo, const QStringList& args) override;

    int getMins(const QString id) const;
};

#endif // STOPPOINTMINS_H
