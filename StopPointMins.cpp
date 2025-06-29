#include "StopPointMins.h"
#include <QDebug>

void StopPointMins::onLine(int /*lineNo*/, const QStringList &args)
{
    _data[args[0].toLower()] = args[1].toInt();
}

int StopPointMins::getMins(const QString id) const
{
    auto it = _data.find(id.toLower());

    if( it == _data.end())
    {

#ifdef Q_OS_WIN
        qDebug() << "[" << id << "] TIME INFO NOT FOUND [" << getFilename() << "]";
#endif
        return -1;
    }

    return *it;
}
